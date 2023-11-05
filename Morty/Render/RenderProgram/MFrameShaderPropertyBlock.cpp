#include "MFrameShaderPropertyBlock.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MShader.h"
#include "Basic/MViewport.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Material/MMaterial.h"
#include "Resource/MMaterialResource.h"

#include "Manager/MAnimationManager.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

void MFrameShaderPropertyBlock::Initialize(MEngine* pEngine)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	m_pMaterial = LoadMaterial(pEngine);
	BindMaterial(m_pMaterial);

	std::shared_ptr<MResource> pBrdfTexture = pResourceSystem->LoadResource("Texture/ibl_brdf_lut.png");

	if (std::shared_ptr<MTextureResource> pTexture = MTypeClass::DynamicCast<MTextureResource>(pBrdfTexture))
	{
		SetBrdfMapTexture(pTexture->GetTextureTemplate());
	}
}

void MFrameShaderPropertyBlock::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;

	m_pMaterial = nullptr;
}


std::shared_ptr<MMaterial> MFrameShaderPropertyBlock::LoadMaterial(MEngine* pEngine) const
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
	std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Deferred/deferred_gbuffer.mps");
	auto pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetCullMode(MECullMode::ECullBack);
	pMaterial->LoadShader(forwardVS);
	pMaterial->LoadShader(forwardPS);

	return pMaterial;
}

void MFrameShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pShaderPropertyBlock = pMaterial->GetFramePropertyBlock()->Clone();

	MORTY_ASSERT(m_pFrameParam = m_pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_FRAME_DATA));
	MORTY_ASSERT(m_pLightParam = m_pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_LIGHT_DATA));
	
	MORTY_ASSERT(m_pShadowTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_SHADOW_MAP));
	MORTY_ASSERT(m_pDiffuseMapTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_IRRADIANCE_MAP));
	MORTY_ASSERT(m_pSpecularMapTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_PREFILTER_MAP));
	MORTY_ASSERT(m_pBrdfMapTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_BRDF_LUT));

	MORTY_ASSERT(m_pAnimationBonesParam = m_pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_MATRIX));
	MORTY_ASSERT(m_pAnimationOffsetParam = m_pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_OFFSET));

	MORTY_ASSERT(m_pRWVoxelTableParam = m_pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_VOXEL_TABLE));
	MORTY_ASSERT(m_pVoxelGITextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::VOXELIZER_VOXEL_TEXTURE_NAME));
}

std::shared_ptr<MShaderPropertyBlock> MFrameShaderPropertyBlock::GetPropertyBlock() const
{
	return { m_pShaderPropertyBlock };
}

void MFrameShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info)
{
	const MScene* pScene = info.pScene;
	if (!pScene) return;


	if (m_pFrameParam)
	{
		MVariantStruct& cFrameStruct = m_pFrameParam->var.GetValue<MVariantStruct>();

		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_VIEW_MATRIX, info.m4CameraTransform.Inverse());
		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_PROJ_MATRIX, info.m4CameraInverseProjection);
		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_INV_CAMERA_PROJ_MATRIX, info.m4CameraInverseProjection.Inverse());

		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_POSITION, info.m4CameraTransform.GetTranslation());
		//FIXME no uniform scale.
		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_DIRECTION, info.m4CameraTransform * Vector3(0, 0, 1));


		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_VIEWPORT_SIZE, Vector2(info.f2ViewportSize));
		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_Z_NEAR_FAR, info.f2CameraNearFar);

		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_TIME_DELTA, info.fDelta);
		cFrameStruct.SetVariant(MShaderPropertyName::FRAME_GAME_TIME, info.fGameTime);

		m_pFrameParam->SetDirty();
	}


	if (m_pLightParam)
	{
		MVariantStruct& cLightStruct = m_pLightParam->var.GetValue<MVariantStruct>();

		if (info.pEnvDiffuseTexture && info.pEnvSpecularTexture)
		{
			m_pDiffuseMapTextureParam->SetTexture(info.pEnvDiffuseTexture);
			m_pDiffuseMapTextureParam->SetDirty();
			m_pSpecularMapTextureParam->SetTexture(info.pEnvSpecularTexture);
			m_pSpecularMapTextureParam->SetDirty();

			cLightStruct.SetVariant(MShaderPropertyName::LIGHT_ENVIRONMENT_MAP_ENABLE, 1);
		}
        else
        {
			cLightStruct.SetVariant(MShaderPropertyName::LIGHT_ENVIRONMENT_MAP_ENABLE, 0);
        }

		cLightStruct.SetVariant(MShaderPropertyName::LIGHT_DIRECTION_LIGHT_ENABLE, 1);
		auto& cDirectionLightStruct = cLightStruct.GetVariant<MVariantStruct>(MShaderPropertyName::LIGHT_DIRECTION_STRUCT_NAME);
		{
			cDirectionLightStruct.SetVariant(MShaderPropertyName::LIGHT_DIRECTION_LIGHT_DIR, info.directionLight.f3LightDirection);
			cDirectionLightStruct.SetVariant(MShaderPropertyName::LIGHT_INTENSITY, info.directionLight.f3LightIntensity);
			cDirectionLightStruct.SetVariant(MShaderPropertyName::LIGHT_DIRECTION_LIGHT_SIZE, info.directionLight.fLightSize);
		}

		size_t nPointLightNum = std::min(size_t(MRenderGlobal::POINT_LIGHT_MAX_NUMBER), info.vPointLight.size());
		cLightStruct.SetVariant(MShaderPropertyName::LIGHT_POINT_COUNT, int(nPointLightNum));
		MVariantArray& vPointLights = cLightStruct.GetVariant<MVariantArray>(MShaderPropertyName::LIGHT_POINT_ARRAY_NAME);
   		for (size_t nPointIdx = 0; nPointIdx < nPointLightNum; ++nPointIdx)
   		{
			MVariantStruct cPointLight = vPointLights.GetVariant<MVariantStruct>(nPointIdx);
			cPointLight.SetVariant(MShaderPropertyName::LIGHT_POINT_POSITION, info.vPointLight[nPointIdx].f3LightPosition);
			cPointLight.SetVariant(MShaderPropertyName::LIGHT_INTENSITY, info.vPointLight[nPointIdx].f3LightIntensity);

			cPointLight.SetVariant(MShaderPropertyName::LIGHT_POINT_CONSTANT, info.vPointLight[nPointIdx].fConstant);
			cPointLight.SetVariant(MShaderPropertyName::LIGHT_POINT_LINEAR, info.vPointLight[nPointIdx].fLinear);
			cPointLight.SetVariant(MShaderPropertyName::LIGHT_POINT_QUADRATIC, info.vPointLight[nPointIdx].fQuadratic);
   		}
/*
   		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
   		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
   		{
   			std::vector<MSpotLightComponent*> vActiveSpotLights(MGlobal::SPOT_LIGHT_MAX_NUMBER);
   			info.pScene->FindActiveSpotLights(info.pCameraSceneComponent->GetWorldPosition(), vActiveSpotLights);
   			varValidSpotLights = 0;
 
   			MVariantArray& vSpotLights = *varSpotLights.GetArray();
   			for (uint32_t i = 0; i < vSpotLights.GetMemberCount(); ++i)
   			{
   				if (MSpotLightComponent* pSpotLightComponent = vActiveSpotLights[i])
   				{
   					MNode* pNode = pSpotLightComponent->GetOwnerNode();
   					MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
 
   					Vector3 f3SpotDirection = pSpotLightComponent->GetWorldDirection();
   					f3SpotDirection.Normalize();
   					MStruct& cSpotLight = *vSpotLights[i].GetStruct();
   					cSpotLight[0] = pSceneComponent->GetWorldPosition();
   					cSpotLight[1] = pSpotLightComponent->GetInnerCutOffRadius();
   					cSpotLight[2] = f3SpotDirection;
   					cSpotLight[3] = pSpotLightComponent->GetOuterCutOffRadius();
   					cSpotLight[4] = pSpotLightComponent->GetColor().ToVector3() * pSpotLightComponent->GetLightIntensity();
 
   					varValidSpotLights = (int)i + 1;
   				}
   				else break;
   			}
   		}
*/

		MVariantArray& cDirLightInvProjArray = cLightStruct.GetVariant<MVariantArray>(MShaderPropertyName::SHADOW_LIGHT_PROJ_MATRIX);
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				cDirLightInvProjArray.SetVariant(nCascadedIdx, info.shadowRenderInfo[nCascadedIdx].m4DirLightInvProj);
			}
		}

		MVariantArray& cSplitDepthArray = cLightStruct.GetVariant<MVariantArray>(MShaderPropertyName::SHADOW_LIGHT_CASCADE_SPLIT);
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				cSplitDepthArray.SetVariant(nCascadedIdx, info.shadowRenderInfo[nCascadedIdx].fSplitRange);
			}
		}

		auto& settingStruct = cLightStruct.GetVariant<MVariantStruct>(MShaderPropertyName::VOXEL_MAP_SETTING);
		MRenderInfo::FillVoxelMapSetting(info.voxelSetting, settingStruct);

		const MVoxelClipmap& clipmap = info.voxelSetting.vClipmap[info.voxelSetting.nClipmapIdx];

		const float fVoxelTableSize = static_cast<float>(info.voxelSetting.nResolution) * clipmap.fVoxelSize;
		const auto m4CameraProj = MRenderSystem::MatrixOrthoOffCenterLH(
			clipmap.f3VoxelOrigin.x,
			clipmap.f3VoxelOrigin.x + fVoxelTableSize,
			clipmap.f3VoxelOrigin.y + fVoxelTableSize,
			clipmap.f3VoxelOrigin.y,
			clipmap.f3VoxelOrigin.z,
			clipmap.f3VoxelOrigin.z + fVoxelTableSize
		);
		

		cLightStruct.SetVariant<Matrix4>(MShaderPropertyName::VOXELIZER_CAMERA_PROJ_MATRIX, m4CameraProj);

		m_pLightParam->SetDirty();
	}

	if (const auto pAnimationManager = pScene->GetManager<MAnimationManager>())
	{
		const auto bufferData = pAnimationManager->GetAnimationBuffer();
		m_pAnimationBonesParam->pBuffer = bufferData.pBonesBuffer;
		m_pAnimationOffsetParam->pBuffer = bufferData.pOffsetBuffer;

		m_pAnimationBonesParam->SetDirty();
		m_pAnimationOffsetParam->SetDirty();
	}


	if (m_pRWVoxelTableParam->pBuffer != info.pVoxelTableBuffer)
	{
		m_pRWVoxelTableParam->pBuffer = info.pVoxelTableBuffer;
		m_pRWVoxelTableParam->SetDirty();
	}

	if (m_pVoxelGITextureParam->pTexture != info.pVoxelGITexture)
	{
		m_pVoxelGITextureParam->pTexture = info.pVoxelGITexture;
		m_pVoxelGITextureParam->SetDirty();
	}
}

void MFrameShaderPropertyBlock::SetShadowMapTexture(std::shared_ptr<MTexture> pTexture)
{
	if (m_pShadowTextureParam && m_pShadowTextureParam->GetTexture() != pTexture)
	{
		m_pShadowTextureParam->SetTexture(pTexture);
	}
}

void MFrameShaderPropertyBlock::SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture)
{
	if (m_pBrdfMapTextureParam && m_pBrdfMapTextureParam->GetTexture() != pTexture)
	{
		m_pBrdfMapTextureParam->SetTexture(pTexture);
	}
}

void MForwardRenderTransparentShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MFrameShaderPropertyBlock::BindMaterial(pMaterial);

	MORTY_ASSERT(m_pTransparentFrontTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0));
	MORTY_ASSERT(m_pTransparentBackTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1));
}
