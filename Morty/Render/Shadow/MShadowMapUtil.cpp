#include "MShadowMapUtil.h"

#include "Basic/MBuffer.h"
#include "Basic/MViewport.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"

#include "Batch/BatchGroup/MInstanceBatchGroup.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "VXGI/MVoxelMapUtil.h"

#define SHADOW_VIEW_FROM_PCS

using namespace morty;

class MFilterFromCameraFrustum : public IRenderableFilter
{
public:
    MFilterFromCameraFrustum(const MCameraFrustum& frustum, const Vector3& f3Direction)
        : m_frustum(frustum)
        , m_direction(f3Direction)
    {}

    bool Filter(const MMeshInstanceRenderProxy* instance) const override
    {
        const MBoundsAABB& bounds = instance->boundsWithTransform;
        if (MCameraFrustum::EOUTSIDE == m_frustum.ContainTest(bounds, m_direction)) { return false; }

        return true;
    }


    MCameraFrustum m_frustum;
    Vector3        m_direction;
};

class MFilterFromSphere : public IRenderableFilter
{
public:
    MFilterFromSphere(const MBoundsSphere& sphere, const Vector3& f3Direction)
        : m_sphere(sphere)
        , m_direction(f3Direction)
    {}

    bool Filter(const MMeshInstanceRenderProxy* instance) const override
    {
        const MBoundsSphere& bounds = instance->boundsWithTransform.ToSphere();

        const Vector3        proj =
                m_direction.Projection(m_sphere.m_centerPoint - bounds.m_centerPoint) + bounds.m_centerPoint;

        return (proj - m_sphere.m_centerPoint).Length() < (m_sphere.m_radius + bounds.m_radius);
    }


    MBoundsSphere m_sphere;
    Vector3       m_direction;
};

MCascadedArray<MCascadedSplitData> MShadowMapUtil::CascadedSplitCameraFrustum(MViewport* pViewport)
{
    MScene*  pScene        = pViewport->GetScene();
    MEntity* pCameraEntity = pViewport->GetCamera();

    MEntity* pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    if (!pDirectionalLightEntity)
    {
        MORTY_ASSERT(pDirectionalLightEntity);
        return {};
    }

    const MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
    if (!pLightSceneComponent)
    {
        MORTY_ASSERT(pLightSceneComponent);
        return {};
    }

    const MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
    if (!pCameraComponent)
    {
        MORTY_ASSERT(pCameraComponent);
        return {};
    }
    MComponentGroup<MRenderMeshComponent>* pMeshComponentGroup = pScene->FindComponents<MRenderMeshComponent>();
    if (!pMeshComponentGroup)
    {
        MORTY_ASSERT(pMeshComponentGroup);
        return {};
    }

    std::array<MCascadedSplitData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadedData;

    const float                                                            fCascadeSplitLambda = 0.95f;

    const float                                                            fNearZ = pCameraComponent->GetZNear();
    const float                                                            fFarZ  = pCameraComponent->GetZFar();

    const float                                                            fRange = fFarZ - fNearZ;
    const float                                                            fRatio = fFarZ / fNearZ;

    float                                                                  fLastSplit = 0.0f;
    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
    {
        const float p       = (nCascadedIdx + 1) / static_cast<float>(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
        const float log     = fNearZ * std::pow(fRatio, p);
        const float uniform = fNearZ + fRange * p;
        const float d       = fCascadeSplitLambda * (log - uniform) + uniform;

        const float fSplit = (d - fNearZ) / fRange;
        //const float fSplit = float(nCascadedIdx + 1) / (MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
        const float fCascadedTransitionRange = 0.25f;
        float       fTransitionRange         = (fSplit - fLastSplit) * fCascadedTransitionRange;

        vCascadedData[nCascadedIdx].fCascadeSplit    = fSplit;
        vCascadedData[nCascadedIdx].fTransitionRange = fTransitionRange;
        vCascadedData[nCascadedIdx].fNearZ           = fNearZ + fRange * fLastSplit;
        vCascadedData[nCascadedIdx].fFarZ            = fNearZ + fRange * fSplit;
        vCascadedData[nCascadedIdx].fOverFarZ        = fNearZ + fRange * (fSplit + fTransitionRange);
        fLastSplit                                   = fSplit;
    }

    return vCascadedData;
}


MBoundsSphere ConvertSphere(const MBoundsSphere& sphere, Matrix4 mat)
{
    MBoundsSphere result = sphere;

    result.m_centerPoint = mat * result.m_centerPoint;

    return result;
}

MBoundsAABB ConvertAABB(const MBoundsAABB& aabb, Matrix4 mat)
{
    std::vector<Vector3> points(8);
    std::vector<Vector3> convertPoints(8);
    aabb.GetPoints(points);
    for (size_t i = 0; i < 8; ++i) { convertPoints[i] = mat * points[i]; }

    MBoundsAABB result;
    result.SetPoints(convertPoints);
    return result;
}

MCascadedArray<MCascadedShadowRenderData> MShadowMapUtil::CalculateRenderData(
        MViewport*                                pViewport,
        MEntity*                                  pCameraEntity,
        const MCascadedArray<MCascadedSplitData>& vCascadedData,
        const MCascadedArray<MBoundsSphere>&      vCascadedPsrBounds,
        const MCascadedArray<MBoundsAABB>&        vCascadedPscBounds
)
{

    MScene*  pScene = pViewport->GetScene();

    MEntity* pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    if (!pDirectionalLightEntity) { return {}; }

    MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
    if (!pLightSceneComponent)
    {
        MORTY_ASSERT(pLightSceneComponent);
        return {};
    }

    MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
    if (!pCameraComponent) { return {}; }

    const float fNearZ = pCameraComponent->GetZNear();
    const float fFarZ  = pCameraComponent->GetZFar();

    Matrix4     matLight(pLightSceneComponent->GetTransform().GetRotation());
    Matrix4     matLightInv = matLight.Inverse();

    std::array<Matrix4, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> vCascadeProjectionMatrix;
    std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>   vCascadeFrustumWidth;
    std::array<float, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>   vRenderBoundsInLightSpaceMinZ;
    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
    {
        const auto&   cCascadedPsrBoundsInWorldSpace = vCascadedPsrBounds[nCascadedIdx];

        MBoundsSphere cascadedFrustumSphereInLightSpace = cCascadedPsrBoundsInWorldSpace;
        cascadedFrustumSphereInLightSpace.m_centerPoint = matLightInv * cascadedFrustumSphereInLightSpace.m_centerPoint;

        MBoundsAABB cascadedAABBBoundsInLightSpace;
        cascadedAABBBoundsInLightSpace.SetMinMax(
                cascadedFrustumSphereInLightSpace.m_centerPoint -
                        Vector3::Fill(cascadedFrustumSphereInLightSpace.m_radius),
                cascadedFrustumSphereInLightSpace.m_centerPoint +
                        Vector3::Fill(cascadedFrustumSphereInLightSpace.m_radius)
        );

        MBoundsAABB lightFrustumInLightSpace = cascadedAABBBoundsInLightSpace;

#ifdef SHADOW_VIEW_FROM_PCS
        {
            if (vCascadedPscBounds[nCascadedIdx].m_halfLength.Length() > 1e-3)
            {
                MBoundsAABB pscInLightSpace = ConvertAABB(vCascadedPscBounds[nCascadedIdx], matLightInv);

                Vector3     min = lightFrustumInLightSpace.m_minPoint;
                Vector3     max = lightFrustumInLightSpace.m_maxPoint;
                for (size_t nIdx = 0; nIdx < 3; ++nIdx)
                {
                    min.m[nIdx] = (std::max)(min.m[nIdx], pscInLightSpace.m_minPoint.m[nIdx]);
                    max.m[nIdx] = (std::min)(max.m[nIdx], pscInLightSpace.m_maxPoint.m[nIdx]);

                    min.m[nIdx] = (std::min)(min.m[nIdx], max.m[nIdx]);
                }
                min.z = pscInLightSpace.m_minPoint.z;
                max.z = std::min(pscInLightSpace.m_maxPoint.z, max.z);
                lightFrustumInLightSpace.SetMinMax(min, max);

                lightFrustumInLightSpace.m_halfLength.x = lightFrustumInLightSpace.m_halfLength.y =
                        std::max(lightFrustumInLightSpace.m_halfLength.x, lightFrustumInLightSpace.m_halfLength.y);
                lightFrustumInLightSpace.SetMinMax(
                        lightFrustumInLightSpace.m_centerPoint - lightFrustumInLightSpace.m_halfLength,
                        lightFrustumInLightSpace.m_centerPoint + lightFrustumInLightSpace.m_halfLength
                );
            }
        }
#else
        MORTY_UNUSED(vCascadedPscBounds);
#endif

        if (true)
        {//https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps#moving-the-light-in-texel-sized-increments

            float   fCascadedFrustumSphereLength = cCascadedPsrBoundsInWorldSpace.m_radius * 2.0f;
            Vector3 cameraFrustumLength          = Vector3::Fill(fCascadedFrustumSphereLength);
            Vector2 worldUnitsPerTexel           = {
                    cameraFrustumLength.x / MRenderGlobal::SHADOW_TEXTURE_SIZE,
                    cameraFrustumLength.y / MRenderGlobal::SHADOW_TEXTURE_SIZE,
            };

            Vector3 min    = lightFrustumInLightSpace.m_minPoint;
            Vector3 max    = lightFrustumInLightSpace.m_maxPoint;
            Vector3 length = max - min;
            min.x          = std::floor(min.x / worldUnitsPerTexel.x) * worldUnitsPerTexel.x;
            min.y          = std::floor(min.y / worldUnitsPerTexel.y) * worldUnitsPerTexel.y;
            length.x       = std::ceil(length.x / worldUnitsPerTexel.x) * worldUnitsPerTexel.x;
            length.y       = std::ceil(length.y / worldUnitsPerTexel.y) * worldUnitsPerTexel.y;
            max.x          = min.x + length.x;
            max.y          = min.y + length.y;
            lightFrustumInLightSpace.SetMinMax(min, max);
        }

        //max bounds
        float fLightProjectionLeft   = lightFrustumInLightSpace.m_minPoint.x;
        float fLightProjectionRight  = lightFrustumInLightSpace.m_maxPoint.x;
        float fLightProjectionTop    = lightFrustumInLightSpace.m_maxPoint.y;
        float fLightProjectionBottom = lightFrustumInLightSpace.m_minPoint.y;
        float fLightProjectionBack   = lightFrustumInLightSpace.m_minPoint.z;
        float fLightProjectionFront  = lightFrustumInLightSpace.m_maxPoint.z;

        vCascadeProjectionMatrix[nCascadedIdx] = MRenderSystem::MatrixOrthoOffCenterLH(
                fLightProjectionLeft,
                fLightProjectionRight,
                fLightProjectionTop,
                fLightProjectionBottom,
                fLightProjectionBack,
                fLightProjectionFront
        );

        vCascadeFrustumWidth[nCascadedIdx]          = fLightProjectionRight - fLightProjectionLeft;
        vRenderBoundsInLightSpaceMinZ[nCascadedIdx] = fLightProjectionBack;
    }

    float fMinLightSpaceZValue = FLT_MAX;
    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
    {
        if (fMinLightSpaceZValue > vRenderBoundsInLightSpaceMinZ[nCascadedIdx])
        {
            fMinLightSpaceZValue = vRenderBoundsInLightSpaceMinZ[nCascadedIdx];
        }
    }

    std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> cRenderData;
    for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
    {
        const Vector4 v4NearZPositionNDCSpace =
                vCascadeProjectionMatrix[nCascadedIdx] * Vector4(0.0f, 0.0f, fMinLightSpaceZValue, 1.0f);

        cRenderData[nCascadedIdx].fSplitRange.x = fNearZ + (fFarZ - fNearZ) * vCascadedData[nCascadedIdx].fCascadeSplit;
        cRenderData[nCascadedIdx].fSplitRange.y =
                fNearZ + (fFarZ - fNearZ
                         ) * (vCascadedData[nCascadedIdx].fCascadeSplit + vCascadedData[nCascadedIdx].fTransitionRange);
        cRenderData[nCascadedIdx].fSplitRange.z     = vCascadeFrustumWidth[nCascadedIdx];
        cRenderData[nCascadedIdx].fSplitRange.w     = v4NearZPositionNDCSpace.z / v4NearZPositionNDCSpace.w;
        cRenderData[nCascadedIdx].m4DirLightInvProj = vCascadeProjectionMatrix[nCascadedIdx] * matLightInv;
    }

    return cRenderData;
}

MCascadedArray<MBoundsSphere> MShadowMapUtil::GetCameraFrustumBounds(
        MViewport*                                pViewport,
        const MCascadedArray<MCascadedSplitData>& vCascadedSplitData
)
{
    MScene*  pScene        = pViewport->GetScene();
    MEntity* pCameraEntity = pViewport->GetCamera();

    MEntity* pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    if (!pDirectionalLightEntity) { return {}; }

    MSceneComponent* pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
    if (!pLightSceneComponent)
    {
        MORTY_ASSERT(pLightSceneComponent);
        return {};
    }

    MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
    if (!pCameraComponent) { return {}; }

    const float                   fNearZ = pCameraComponent->GetZNear();
    const float                   fFarZ  = pCameraComponent->GetZFar();

    MCascadedArray<MBoundsSphere> vResult;

    for (size_t nCascadedIdx = 0; nCascadedIdx < vCascadedSplitData.size(); ++nCascadedIdx)
    {
        float fLastSplitDist = nCascadedIdx ? vCascadedSplitData[nCascadedIdx - 1].fCascadeSplit : 0.0f;
        float fCurrSplitDist =
                (vCascadedSplitData[nCascadedIdx].fCascadeSplit + vCascadedSplitData[nCascadedIdx].fTransitionRange);

        float                fCascadedNearZ = fNearZ + (fFarZ - fNearZ) * fLastSplitDist;
        float                fCascadedFarZ  = fNearZ + (fFarZ - fNearZ) * fCurrSplitDist;

        std::vector<Vector3> vCascadedFrustumPointsInWorldSpace(8);
        MRenderSystem::GetCameraFrustumPoints(
                pCameraEntity,
                pViewport->GetSize(),
                fCascadedNearZ,
                fCascadedFarZ,
                vCascadedFrustumPointsInWorldSpace
        );
        MBoundsSphere cCascadedFrustumSphereInWorldSpace;
        cCascadedFrustumSphereInWorldSpace.SetPoints(vCascadedFrustumPointsInWorldSpace);

        vResult[nCascadedIdx] = cCascadedFrustumSphereInWorldSpace;
    }

    return vResult;
}

MCascadedArray<std::unique_ptr<class IRenderableFilter>> MShadowMapUtil::GetCameraFrustumCullingFilter(
        MViewport*                                pViewport,
        const MCascadedArray<MCascadedSplitData>& vCascadedSplitData
)
{
    MScene*                 pScene        = pViewport->GetScene();
    MEntity*                pCameraEntity = pViewport->GetCamera();

    MEntity*                pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    const MCameraComponent* pCameraComponent        = pCameraEntity->GetComponent<MCameraComponent>();
    if (!pCameraComponent)
    {
        MORTY_ASSERT(pCameraComponent);
        return {};
    }
    MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
    if (!pCameraSceneComponent)
    {
        MORTY_ASSERT(pCameraSceneComponent);
        return {};
    }

    auto pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
    if (!pLightSceneComponent)
    {
        MORTY_ASSERT(pLightSceneComponent);
        return {};
    }

    Vector3                                            v3LightDirection = pLightSceneComponent->GetForward();

    MCascadedArray<std::unique_ptr<IRenderableFilter>> vCascadedFilter;

    std::transform(
            vCascadedSplitData.begin(),
            vCascadedSplitData.end(),
            vCascadedFilter.begin(),
            [&](const MCascadedSplitData& ele) {
                MCameraFrustum cCameraFrustum;

                Matrix4        m4CameraInvProj = MRenderSystem::GetCameraInverseProjection(
                        pCameraComponent,
                        pCameraSceneComponent,
                        pViewport->GetSize().x,
                        pViewport->GetSize().y,
                        ele.fNearZ,
                        ele.fOverFarZ
                );
                cCameraFrustum.UpdateFromCameraInvProj(m4CameraInvProj);

                return std::make_unique<MFilterFromCameraFrustum>(cCameraFrustum, v3LightDirection);
            }
    );

    return vCascadedFilter;
}

MCascadedArray<MBoundsSphere>
MShadowMapUtil::GetVoxelMapBounds(MViewport* pViewport, const MCascadedArray<MCascadedSplitData>& vCascadedSplitData)
{
    MEntity*         pCameraEntity         = pViewport->GetCamera();
    MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
    if (!pCameraSceneComponent)
    {
        MORTY_ASSERT(pCameraSceneComponent);
        return {};
    }

    const Vector3                 f3CameraPosition = pCameraSceneComponent->GetWorldPosition();
    const size_t                  nCascadedNum     = vCascadedSplitData.size();

    MCascadedArray<MBoundsSphere> vResult;

    const auto clipmap         = MVoxelMapUtil::GetClipMap(f3CameraPosition, MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM - 1);
    const auto clipmapBounding = MVoxelMapUtil::GetClipMapBounding(clipmap);

    for (size_t nCascadedIdx = 0; nCascadedIdx < nCascadedNum; ++nCascadedIdx)
    {
        float fCurrSplitDist =
                (vCascadedSplitData[nCascadedIdx].fCascadeSplit + vCascadedSplitData[nCascadedIdx].fTransitionRange);

        auto cascadedBounding = clipmapBounding;
        cascadedBounding.SethalfLength(cascadedBounding.m_halfLength * fCurrSplitDist);

        vResult[nCascadedIdx] = cascadedBounding.ToSphere();
    }


    return vResult;
}

MCascadedArray<std::unique_ptr<class IRenderableFilter>>
MShadowMapUtil::GetBoundsCullingFilter(MViewport* pViewport, const MCascadedArray<MBoundsSphere>& vBoundsSphere)
{
    MScene*    pScene                  = pViewport->GetScene();
    MEntity*   pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();

    const auto pLightSceneComponent = pDirectionalLightEntity->GetComponent<MSceneComponent>();
    if (!pLightSceneComponent)
    {
        MORTY_ASSERT(pLightSceneComponent);
        return {};
    }

    const Vector3                                      v3LightDirection = pLightSceneComponent->GetForward();

    MCascadedArray<std::unique_ptr<IRenderableFilter>> vCascadedFilter;

    std::transform(
            vBoundsSphere.begin(),
            vBoundsSphere.end(),
            vCascadedFilter.begin(),
            [&](const MBoundsSphere& sphere) { return std::make_unique<MFilterFromSphere>(sphere, v3LightDirection); }
    );

    return vCascadedFilter;
}
