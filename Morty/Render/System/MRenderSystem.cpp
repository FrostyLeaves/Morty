#include "System/MRenderSystem.h"


#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "RHI/Vulkan/MVulkanRenderCommand.h"
#include "Scene/MScene.h"

#include "Component/MRenderMeshComponent.h"

#include "Utility/MGlobal.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MSceneComponent.h"
#include "System/MObjectSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderSystem, MISystem)

MRenderSystem::MRenderSystem()
    : MISystem()
    , m_device(nullptr)
{}

MRenderSystem::~MRenderSystem() {}

void MRenderSystem::Update(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    m_device->Update();
}

MIDevice* MRenderSystem::GetDevice() const { return m_device; }

void      MRenderSystem::Initialize()
{
    if (!m_device)
    {
        m_device = new MVulkanDevice();
        m_device->SetEngine(GetEngine());
        m_device->Initialize();
    }
}

void MRenderSystem::Release()
{
    if (m_device)
    {
        m_device->Release();
        delete m_device;
        m_device = nullptr;
    }
}

void MRenderSystem::ResizeFrameBuffer(MRenderPass& renderpass, const Vector2i& n2Size)
{
    for (MRenderTarget& tex: renderpass.m_renderTarget.backTargets)
    {
        if (tex.pTexture->GetSize2D() != n2Size) { tex.pTexture->Resize(GetDevice(), n2Size); }
    }

    if (std::shared_ptr<MTexture> pDepthTexture = renderpass.GetDepthTexture())
    {
        if (pDepthTexture->GetSize2D() != n2Size) { pDepthTexture->Resize(GetDevice(), n2Size); }
    }

    if (auto pShadingRate = renderpass.GetShadingRateTexture())
    {
        Vector2i n2TexelSize       = m_device->GetShadingRateTextureTexelSize();
        Vector2i n2ShadingRateSize = {};
        n2ShadingRateSize.x        = n2Size.x / n2TexelSize.x + ((n2Size.x % n2TexelSize.x) != 0);
        n2ShadingRateSize.y        = n2Size.y / n2TexelSize.y + ((n2Size.y % n2TexelSize.y) != 0);

        if (pShadingRate->GetSize2D() != n2ShadingRateSize) { pShadingRate->Resize(GetDevice(), n2ShadingRateSize); }
    }

    if (renderpass.GetFrameBufferSize() != n2Size) { renderpass.Resize(GetDevice()); }
}

void MRenderSystem::ReleaseRenderpass(MRenderPass& renderpass, bool bClearTexture)
{
    if (bClearTexture)
    {
        for (MRenderTarget& tex: renderpass.m_renderTarget.backTargets)
        {
            tex.pTexture->DestroyBuffer(GetDevice());
            tex.pTexture = nullptr;
        }

        if (std::shared_ptr<MTexture> pDepthTexture = renderpass.GetDepthTexture())
        {
            pDepthTexture->DestroyBuffer(GetDevice());
            pDepthTexture = nullptr;
        }
    }

    renderpass.m_renderTarget.backTargets.clear();
    renderpass.SetDepthTexture(nullptr, {});

    renderpass.DestroyBuffer(GetDevice());
}

MCameraFrustum MRenderSystem::GetCameraFrustum(
        MViewport*        pViewport,
        MCameraComponent* pCameraComponent,
        MSceneComponent*  pSceneComponent
)
{
    MCameraFrustum cameraFrustum;
    const Matrix4  matCameraInverseProj = GetCameraInverseProjection(pViewport, pCameraComponent, pSceneComponent);
    cameraFrustum.UpdateFromCameraInvProj(matCameraInverseProj);

    return cameraFrustum;
}

Matrix4 MRenderSystem::GetCameraViewMatrix(MSceneComponent* pSceneComponent)
{
    return pSceneComponent->GetWorldTransform().Inverse();
}

Matrix4 MRenderSystem::GetPerspectiveProjectionMatrix(
        const float fViewportWidth,
        const float fViewportHeight,
        const float fNear,
        const float fRar,
        const float fFov
)
{
    const Matrix4 m4Projection =
            MRenderSystem::MatrixPerspectiveFovLH(fFov, fViewportWidth / fViewportHeight, fNear, fRar);

    return m4Projection;
}

Matrix4
MRenderSystem::GetOrthoOffProjectionMatrix(const float fWidth, const float fHeight, const float fNear, const float fRar)
{
    const Matrix4 m4Projection = MRenderSystem::MatrixOrthoOffCenterLH(
            -fWidth * 0.5f,
            fWidth * 0.5f,
            fHeight * 0.5f,
            -fHeight * 0.5f,
            fNear,
            fRar
    );

    return m4Projection;
}

Matrix4 MRenderSystem::GetCameraInverseProjection(
        const MViewport*        pViewport,
        const MCameraComponent* pCameraComponent,
        MSceneComponent*        pSceneComponent
)
{
    return GetCameraInverseProjection(
            pCameraComponent,
            pSceneComponent,
            pViewport->GetSize().x,
            pViewport->GetSize().y,
            pCameraComponent->GetZNear(),
            pCameraComponent->GetZFar()
    );
}

Matrix4 MRenderSystem::GetCameraProjectionMatrix(
        const MCameraComponent* pCameraComponent,
        float                   fViewWidth,
        float                   fViewHeight,
        float                   fZNear,
        float                   fZFar
)
{
    //Update Camera and Projection Matrix.
    const Matrix4 m4Projection =
            pCameraComponent->GetCameraType() == MECameraType::EPerspective
                    ? GetPerspectiveProjectionMatrix(fViewWidth, fViewHeight, fZNear, fZFar, pCameraComponent->GetFov())
                    : GetOrthoOffProjectionMatrix(
                              pCameraComponent->GetWidth(),
                              pCameraComponent->GetWidth(),
                              fZNear,
                              fZFar
                      );

    return m4Projection;
}

Matrix4 MRenderSystem::GetCameraInverseProjection(
        const MCameraComponent* pCameraComponent,
        MSceneComponent*        pSceneComponent,
        float                   fViewWidth,
        float                   fViewHeight,
        float                   fZNear,
        float                   fZFar
)
{
    //Update Camera and Projection Matrix.
    const Matrix4 m4Projection = GetCameraProjectionMatrix(pCameraComponent, fViewWidth, fViewHeight, fZNear, fZFar);

    return m4Projection * pSceneComponent->GetWorldTransform().Inverse();
}

void MRenderSystem::GetCameraFrustumPoints(
        MEntity*              pCamera,
        const Vector2i&       v2ViewportSize,
        const float&          fZNear,
        const float&          fZFar,
        std::vector<Vector3>& vPoints
)
{
    GetCameraFrustumPoints(
            pCamera,
            v2ViewportSize,
            fZNear,
            fZFar,
            vPoints[0],
            vPoints[1],
            vPoints[2],
            vPoints[3],
            vPoints[4],
            vPoints[5],
            vPoints[6],
            vPoints[7]
    );
}

void MRenderSystem::GetCameraFrustumPoints(
        MEntity*        pCamera,
        const Vector2i& v2ViewportSize,
        const float&    fZNear,
        const float&    fZFar,
        Vector3&        v3NearTopLeft,
        Vector3&        v3NearTopRight,
        Vector3&        v3NearBottomRight,
        Vector3&        v3NearBottomLeft,
        Vector3&        v3FarTopLeft,
        Vector3&        v3FarTopRight,
        Vector3&        v3FarBottomRight,
        Vector3&        v3FarBottomLeft
)
{

    MCameraComponent* pCameraComponent = pCamera->GetComponent<MCameraComponent>();
    if (nullptr == pCameraComponent) return;

    MSceneComponent* pSceneComponent = pCamera->GetComponent<MSceneComponent>();
    if (nullptr == pSceneComponent) return;

    if (MECameraType::EPerspective == pCameraComponent->GetCameraType())
    {
        const float fAspect            = float(v2ViewportSize.x) / float(v2ViewportSize.y);
        const float fHalfHeightDivideZ = (pCameraComponent->GetFov() * 0.5f * M_PI / 180.0f);
        const float fHalfWidthDivideZ  = fHalfHeightDivideZ * fAspect;

        Matrix4     localToWorld = pSceneComponent->GetWorldTransform();

        v3NearTopLeft     = localToWorld * (Vector3(-fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZNear);
        v3NearTopRight    = localToWorld * (Vector3(+fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZNear);
        v3NearBottomLeft  = localToWorld * (Vector3(-fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZNear);
        v3NearBottomRight = localToWorld * (Vector3(+fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZNear);

        v3FarTopLeft     = localToWorld * (Vector3(-fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZFar);
        v3FarTopRight    = localToWorld * (Vector3(+fHalfWidthDivideZ, +fHalfHeightDivideZ, 1) * fZFar);
        v3FarBottomLeft  = localToWorld * (Vector3(-fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZFar);
        v3FarBottomRight = localToWorld * (Vector3(+fHalfWidthDivideZ, -fHalfHeightDivideZ, 1) * fZFar);
    }
    else
    {
        float   fHalfWidth  = pCameraComponent->GetWidth() * 0.5f;
        float   fHalfHeight = pCameraComponent->GetHeight() * 0.5f;

        Matrix4 localToWorld = pSceneComponent->GetWorldTransform();

        v3NearTopLeft     = localToWorld * Vector3(-fHalfWidth, +fHalfHeight, fZNear);
        v3NearTopRight    = localToWorld * Vector3(+fHalfWidth, +fHalfHeight, fZNear);
        v3NearBottomLeft  = localToWorld * Vector3(-fHalfWidth, -fHalfHeight, fZNear);
        v3NearBottomRight = localToWorld * Vector3(+fHalfWidth, -fHalfHeight, fZNear);

        v3FarTopLeft     = localToWorld * Vector3(-fHalfWidth, +fHalfHeight, fZFar);
        v3FarTopRight    = localToWorld * Vector3(+fHalfWidth, +fHalfHeight, fZFar);
        v3FarBottomLeft  = localToWorld * Vector3(-fHalfWidth, -fHalfHeight, fZFar);
        v3FarBottomRight = localToWorld * Vector3(+fHalfWidth, -fHalfHeight, fZFar);
    }
}

Matrix4 MRenderSystem::MatrixPerspectiveFovLH(
        const float& fFov,
        const float& fScreenAspect,
        const float& fScreenNear,
        const float& fScreenFar
)
{
    Matrix4     mProjMatrix;

    const float angle = (fFov * 0.5f) * M_PI / 180.0f;
    const float s1    = 1 / ((float) tan(angle) * fScreenAspect);
    const float s2    = 1 / tan(angle);
    const float a     = fScreenFar / (fScreenFar - fScreenNear);
    const float b     = -(fScreenNear * fScreenFar) / (fScreenFar - fScreenNear);

    mProjMatrix.m[0][0] = s1;
    mProjMatrix.m[1][1] = s2;
    mProjMatrix.m[2][2] = a;
    mProjMatrix.m[2][3] = b;
    mProjMatrix.m[3][2] = 1.0f;

    return mProjMatrix;
}

Matrix4 MRenderSystem::MatrixOrthoOffCenterLH(
        const float& fLeft,
        const float& fRight,
        const float& fTop,
        const float& fBottom,
        const float& fNear,
        const float& fFar
)
{
    //warning, (fLeft >> fRight) and (fTop >> fBottom) and (fFar >> fNear)
    return Matrix4(
            2 / (fRight - fLeft),
            0,
            0,
            (fLeft + fRight) / (fLeft - fRight),
            0,
            2 / (fTop - fBottom),
            0,
            (fTop + fBottom) / (fBottom - fTop),
            0,
            0,
            1 / (fFar - fNear),
            fNear / (fNear - fFar),
            0,
            0,
            0,
            1
    );
}
