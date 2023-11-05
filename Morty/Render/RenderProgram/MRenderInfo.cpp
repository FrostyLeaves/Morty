#include "MRenderInfo.h"


#include "Scene/MScene.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "System/MRenderSystem.h"

MRenderInfo MRenderInfo::CreateFromViewport(MViewport* pViewport)
{
    MScene* pScene = pViewport->GetScene();
    MORTY_ASSERT(pScene);

    MEntity* pCamera = pScene->FindFirstEntityByComponent<MCameraComponent>();
    MORTY_ASSERT(pCamera);

    MSceneComponent* pCameraSceneComponent = pCamera->GetComponent<MSceneComponent>();
    MORTY_ASSERT(pCameraSceneComponent);

    MCameraComponent* pCameraComponent = pCamera->GetComponent<MCameraComponent>();
    MORTY_ASSERT(pCameraComponent);


    MRenderInfo info;
    info.pScene = pScene;

    info.f2ViewportLeftTop = pViewport->GetLeftTop();
    info.f2ViewportSize = pViewport->GetSize();

    info.m4CameraTransform = pCameraSceneComponent->GetWorldTransform();
    info.f2CameraNearFar = pCameraComponent->GetZNearFar();

    const auto m4CameraInverseProj = MRenderSystem::GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent);
    info.m4CameraInverseProjection = m4CameraInverseProj;
    info.cameraFrustum.UpdateFromCameraInvProj(m4CameraInverseProj);


    if (MEntity* pDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>())
    {
        MSceneComponent* pDirectionalLightSceneComponent = pDirectionalLight->GetComponent<MSceneComponent>();
        MORTY_ASSERT(pDirectionalLightSceneComponent);

        MDirectionalLightComponent* pDirectionalLightComponent = pDirectionalLight->GetComponent<MDirectionalLightComponent>();
        MORTY_ASSERT(pDirectionalLightComponent);

        info.directionLight.f3LightDirection = pDirectionalLightSceneComponent->GetForward();
        info.directionLight.f3LightIntensity = pDirectionalLightComponent->GetColor().ToVector3() * pDirectionalLightComponent->GetLightIntensity();
        info.directionLight.fLightSize = pDirectionalLightComponent->GetLightSize();
    }

    if (MEntity* pSkyBoxEntity = pScene->FindFirstEntityByComponent<MSkyBoxComponent>())
    {
        MSkyBoxComponent* pSkyBoxComponent = pSkyBoxEntity->GetComponent<MSkyBoxComponent>();
        info.pEnvDiffuseTexture = pSkyBoxComponent->GetDiffuseTexture();
        info.pEnvSpecularTexture = pSkyBoxComponent->GetSpecularTexture();
    }


    MComponentGroup<MPointLightComponent>* pComponentGroup = pScene->FindComponents<MPointLightComponent>();
    for (const MPointLightComponent& lightComponent : pComponentGroup->m_vComponents)
    {
        if (!lightComponent.IsValid())
        {
            continue;
        }

        const MPointLightComponent* pPointLightComponent = &lightComponent;

        MEntity* pEntity = pPointLightComponent->GetEntity();
        MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

        if (!pSceneComponent)
        {
            continue;
        }

        MPointLightData lightData;
        lightData.f3LightPosition = pSceneComponent->GetWorldPosition();
        lightData.f3LightIntensity = pPointLightComponent->GetColor().ToVector3() * pPointLightComponent->GetLightIntensity();
        lightData.fConstant = pPointLightComponent->GetConstant();
        lightData.fLinear = pPointLightComponent->GetLinear();
        lightData.fQuadratic = pPointLightComponent->GetQuadratic();

        info.vPointLight.push_back(lightData);
    }


    //TODO: remove test code.
    const uint32_t nVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;
    const float fBasicVoxelSize = 0.125f;

    MVoxelMapSetting voxelSetting;
    voxelSetting.nResolution = nVoxelTableSize;
    voxelSetting.nViewportSize = MRenderGlobal::VOXEL_VIEWPORT_SIZE;

    for (size_t nClipIdx = 0; nClipIdx < MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM; ++nClipIdx)
    {
        const float fVoxelSize = fBasicVoxelSize * std::powf(2.0f, nClipIdx);
        Vector3 f3Origin = info.m4CameraTransform.GetTranslation() - nVoxelTableSize * fVoxelSize * 0.5f;
        const Vector3i n3FloorPosition = MMath::Floor(f3Origin / fVoxelSize);
        f3Origin = Vector3(n3FloorPosition.x, n3FloorPosition.y, n3FloorPosition.z) * fVoxelSize;

        voxelSetting.vClipmap[nClipIdx] =
        {
            f3Origin,
            fVoxelSize
        };
    }

    info.voxelSetting = voxelSetting;



    return info;
}

void MRenderInfo::FillVoxelMapSetting(const MVoxelMapSetting& setting, MVariantStruct& output)
{
    auto& vClipmap = output.GetVariant<MVariantArray>(MShaderPropertyName::VOXEL_MAP_CLIPMAP_ARRAY);
    for (size_t nIdx = 0; nIdx < MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM; ++nIdx)
    {
        auto& clipmap = vClipmap[nIdx].GetValue<MVariantStruct>();
        clipmap.SetVariant(MShaderPropertyName::VOXEL_MAP_ORIGIN, setting.vClipmap[nIdx].f3VoxelOrigin);
        clipmap.SetVariant(MShaderPropertyName::VOXEL_MAP_STEP_SIZE, setting.vClipmap[nIdx].fVoxelSize);
    }

    output.SetVariant(MShaderPropertyName::VOXEL_MAP_RESOLUTION, setting.nResolution);
    output.SetVariant(MShaderPropertyName::VOXEL_MAP_VIEWPORT_SIZE, setting.nViewportSize);
    output.SetVariant(MShaderPropertyName::VOXEL_MAP_CLIPMAP_INDEX, setting.nClipmapIdx);
}
