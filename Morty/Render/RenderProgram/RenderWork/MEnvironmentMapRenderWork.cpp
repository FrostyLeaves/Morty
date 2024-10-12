#include "MEnvironmentMapRenderWork.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Math/MMath.h"
#include "RHI/MRenderCommand.h"
#include "Scene/MEntity.h"

#include "Material/MMaterial.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MMeshResource.h"
#include "Resource/MTextureResource.h"

#include "Component/MSkyBoxComponent.h"
#include "Resource/MMeshResourceUtil.h"

using namespace morty;

const int   SpecularMipmapCount    = 7;
const float EnvironmentTextureSize = 128.0f;

MORTY_CLASS_IMPLEMENT(MEnvironmentMapRenderWork, MObject)

MEnvironmentMapRenderWork::MEnvironmentMapRenderWork()
    : MObject()
    , m_updateNextFrame(true)
    , m_DiffuseMaterial(nullptr)
    , m_specularMaterial()
    , m_cubeMesh(nullptr)
{}

void MEnvironmentMapRenderWork::OnCreated()
{
    Super::OnCreated();
    InitializeResource();
    InitializeMaterial();
    InitializeRenderPass();
}

void MEnvironmentMapRenderWork::OnDelete()
{
    ReleaseRenderPass();
    ReleaseMaterial();
    ReleaseResource();
    Super::OnDelete();
}

void MEnvironmentMapRenderWork::MarkUpdateEnvironment() { m_updateNextFrame = true; }

void MEnvironmentMapRenderWork::RenderEnvironment(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
    if (!m_updateNextFrame) return;

    if (!pCommand) return;

    if (!pSkyBoxComponent) return;


    RenderDiffuse(pCommand, pSkyBoxComponent);
    RenderSpecular(pCommand, pSkyBoxComponent);

    m_updateNextFrame = false;
}

std::shared_ptr<MResource> MEnvironmentMapRenderWork::GetDiffuseOutputTexture() const
{
    return m_DiffuseEnvironmentMap.GetResource();
}

void MEnvironmentMapRenderWork::RenderDiffuse(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{

    std::shared_ptr<MResource> pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();
    if (m_DiffuseMaterial)
    {
        m_DiffuseMaterial->SetTexture(MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX, pSkyBoxTexture);
    }


    pCommand->BeginRenderPass(&m_DiffuseRenderPass);

    const Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
    const Vector2 v2Size    = Vector2(EnvironmentTextureSize, EnvironmentTextureSize);
    pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
    pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

    pCommand->SetUseMaterial(m_DiffuseMaterial);
    pCommand->DrawMesh(m_cubeMesh->GetMesh());

    pCommand->EndRenderPass();


    if (std::shared_ptr<MTextureResource> pDiffuseTexture = m_DiffuseEnvironmentMap.GetResource<MTextureResource>())
    {
        if (std::shared_ptr<MTexture> pTexture = pDiffuseTexture->GetTextureTemplate())
        {
            pCommand->AddRenderToTextureBarrier({pTexture.get()}, METextureBarrierStage::EPixelShaderSample);
        }

        pSkyBoxComponent->LoadDiffuseEnvResource(pDiffuseTexture);
    }
}

void MEnvironmentMapRenderWork::RenderSpecular(MIRenderCommand* pCommand, MSkyBoxComponent* pSkyBoxComponent)
{
    std::shared_ptr<MResource> pSkyBoxTexture = pSkyBoxComponent->GetSkyBoxResource();

    std::shared_ptr<MTexture>  pSpecularTexture =
            m_SpecularEnvironmentMap.GetResource<MTextureResource>()->GetTextureTemplate();


    for (uint32_t nIdx = 0; nIdx < m_specularRenderPass.size(); ++nIdx)
    {
        if (m_specularMaterial[nIdx])
        {
            m_specularMaterial[nIdx]->SetTexture(MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX, pSkyBoxTexture);
        }

        pCommand->BeginRenderPass(&m_specularRenderPass[nIdx]);

        Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
        Vector2 v2Size    = pSpecularTexture->GetMipmapSize(nIdx);

        pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
        pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

        pCommand->SetUseMaterial(m_specularMaterial[nIdx]);
        pCommand->DrawMesh(m_cubeMesh->GetMesh());

        pCommand->EndRenderPass();
    }

    if (std::shared_ptr<MTextureResource> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>())
    {
        if (std::shared_ptr<MTexture> pTexture = pSpecularTexture->GetTextureTemplate())
        {
            pCommand->AddRenderToTextureBarrier({pTexture.get()}, METextureBarrierStage::EPixelShaderSample);
        }

        pSkyBoxComponent->LoadSpecularEnvResource(pSpecularTexture);
    }
}

void MEnvironmentMapRenderWork::InitializeResource()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    m_cubeMesh = pResourceSystem->CreateResource<MMeshResource>("Environment Draw Mesh");

    m_cubeMesh->Load(MMeshResourceUtil::CreateSphere());

    if (std::shared_ptr<MTextureResource> pDiffuseCubeMapResource = pResourceSystem->CreateResource<MTextureResource>())
    {
        pDiffuseCubeMapResource->CreateCubeMapRenderTarget(
                EnvironmentTextureSize,
                EnvironmentTextureSize,
                4,
                METextureFormat::Float_RGBA16,
                false
        );

        m_DiffuseEnvironmentMap.SetResource(pDiffuseCubeMapResource);
    }

    if (std::shared_ptr<MTextureResource> pSpecularCubeMapResource =
                pResourceSystem->CreateResource<MTextureResource>())
    {
        pSpecularCubeMapResource->CreateCubeMapRenderTarget(
                EnvironmentTextureSize,
                EnvironmentTextureSize,
                4,
                METextureFormat::Float_RGBA16,
                true
        );

        m_SpecularEnvironmentMap.SetResource(pSpecularCubeMapResource);
    }
}

void MEnvironmentMapRenderWork::ReleaseResource()
{
    m_cubeMesh = nullptr;

    m_DiffuseEnvironmentMap.SetResource(nullptr);
    m_SpecularEnvironmentMap.SetResource(nullptr);
}

void MEnvironmentMapRenderWork::InitializeMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    Matrix4          m4Projection = MRenderSystem::MatrixPerspectiveFovLH(90.0f, 1.0f, 0.1f, 100.0f);

    Matrix4          vCmaeraView[6] = {
            MMath::LookAt(Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f)),
            MMath::LookAt(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f)),
            MMath::LookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f)),
            MMath::LookAt(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 1.0f, 0.0f)),
    };

    for (uint32_t i = 0; i < 6; ++i) { vCmaeraView[i] = m4Projection * vCmaeraView[i]; }

    auto pDiffuseTemplate         = pResourceSystem->CreateResource<MMaterialTemplate>("Diffuse CubeMap Material");
    std::shared_ptr<MResource> vs = pResourceSystem->LoadResource("Shader/Lighting/ibl_map.mvs");
    std::shared_ptr<MResource> diffuseps = pResourceSystem->LoadResource("Shader/Lighting/diffuse_map.mps");
    pDiffuseTemplate->LoadShader(vs);
    pDiffuseTemplate->LoadShader(diffuseps);
    pDiffuseTemplate->SetCullMode(MECullMode::ECullFront);

    m_DiffuseMaterial = MMaterial::CreateMaterial(pDiffuseTemplate);

    if (const std::shared_ptr<MShaderPropertyBlock>& pParams = m_DiffuseMaterial->GetMaterialPropertyBlock())
    {
        MVariantStruct& matrix = pParams->m_params[0]->var.GetValue<MVariantStruct>();
        {
            MVariantArray& mvp = matrix.GetVariant<MVariantArray>(MShaderPropertyName::ENVIRONMENT_IBL_MVP_MATRIX);
            for (uint32_t i = 0; i < 6; ++i) { mvp.SetVariant(i, vCmaeraView[i]); }

            pParams->m_params[0]->SetDirty();
        }
    }


    std::shared_ptr<MResource> specularps = pResourceSystem->LoadResource("Shader/Lighting/specular_map.mps");
    auto pSpecularTemplate = pResourceSystem->CreateResource<MMaterialTemplate>(MString("Specular CubeMap Material"));
    pSpecularTemplate->LoadShader(vs);
    pSpecularTemplate->LoadShader(specularps);
    pSpecularTemplate->SetCullMode(MECullMode::ECullFront);

    m_specularMaterial.resize(SpecularMipmapCount);
    for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
    {
        m_specularMaterial[nMipmap] = MMaterial::CreateMaterial(pSpecularTemplate);

        if (const std::shared_ptr<MShaderPropertyBlock>& pParams =
                    m_specularMaterial[nMipmap]->GetMaterialPropertyBlock())
        {
            {
                MVariantStruct& matrix = pParams->m_params[0]->var.GetValue<MVariantStruct>();
                MVariantArray&  mvp = matrix.GetVariant<MVariantArray>(MShaderPropertyName::ENVIRONMENT_IBL_MVP_MATRIX);

                for (uint32_t i = 0; i < 6; ++i) { mvp.SetVariant(i, vCmaeraView[i]); }

                pParams->m_params[0]->SetDirty();
            }

            {
                MVariantStruct& matrix = pParams->m_params[1]->var.GetValue<MVariantStruct>();
                matrix.SetVariant<float>(
                        MShaderPropertyName::ENVIRONMENT_IBL_ROUGHNESS,
                        (float) nMipmap / (float) (SpecularMipmapCount)
                );
                pParams->m_params[1]->SetDirty();
            }
        }
    }
}

void MEnvironmentMapRenderWork::ReleaseMaterial()
{
    if (m_DiffuseMaterial) { m_DiffuseMaterial = nullptr; }

    for (std::shared_ptr<MMaterial> pMaterial: m_specularMaterial) { pMaterial = nullptr; }
    m_specularMaterial.clear();
}

void MEnvironmentMapRenderWork::InitializeRenderPass()
{
#if MORTY_DEBUG
    m_DiffuseRenderPass.m_strDebugName = "EnvironmentMap Diffuse";
#endif

    MRenderSystem*                    pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    std::shared_ptr<MTextureResource> pDiffuseTexture  = m_DiffuseEnvironmentMap.GetResource<MTextureResource>();
    std::shared_ptr<MTextureResource> pSpecularTexture = m_SpecularEnvironmentMap.GetResource<MTextureResource>();

    m_DiffuseRenderPass.AddBackTexture(pDiffuseTexture->GetTextureTemplate(), {true, MColor::Black_T});

    m_specularRenderPass.resize(SpecularMipmapCount);
    for (uint32_t nMipmap = 0; nMipmap < SpecularMipmapCount; ++nMipmap)
    {
        m_specularRenderPass[nMipmap].SetViewportNum(6);
        m_specularRenderPass[nMipmap].AddBackTexture(
                pSpecularTexture->GetTextureTemplate(),
                {true, MColor::Black_T, nMipmap}
        );
        m_specularRenderPass[nMipmap].GenerateBuffer(pRenderSystem->GetDevice());
    }

    m_DiffuseRenderPass.SetViewportNum(6);
    m_DiffuseRenderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void MEnvironmentMapRenderWork::ReleaseRenderPass()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_DiffuseRenderPass.DestroyBuffer(pRenderSystem->GetDevice());

    for (MRenderPass& renderpass: m_specularRenderPass) { renderpass.DestroyBuffer(pRenderSystem->GetDevice()); }

    m_specularRenderPass.clear();
}
