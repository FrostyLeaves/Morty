#include "MFrameShaderPropertyBlock.h"

#include "MRenderModule.h"
#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Shader/MShader.h"
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
#include "Render/MMaterialName.h"
#include "Resource/MMaterialTemplateResource.h"

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

	if (auto pNoiseResource = pResourceSystem->LoadResource(MRenderModule::NoiseTexture))
	{
		auto pNoiseTexture = pNoiseResource->DynamicCast<MTextureResource>();
		if (pNoiseTexture)
		{
			m_pShaderPropertyBlock->SetTexture(MShaderPropertyName::TEXTURE_NOISE_TEX, pNoiseTexture->GetTextureTemplate());
		}
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

	const auto pMaterialTemplate = pResourceSystem->LoadResource(MMaterialName::FRAME_DEFAULT);
	return MMaterial::CreateMaterial(pMaterialTemplate);
}

void MFrameShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pShaderPropertyBlock = MMaterialTemplate::CreateFramePropertyBlock(pMaterial->GetShaderProgram());

	for (const auto& pDecorator : m_vPropertyUpdateDecorator)
	{
		pDecorator->BindMaterial(m_pShaderPropertyBlock);
	}

	
	MORTY_ASSERT(m_pBrdfMapTextureParam = m_pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_BRDF_LUT));

}

std::shared_ptr<MShaderPropertyBlock> MFrameShaderPropertyBlock::GetPropertyBlock() const
{
	return { m_pShaderPropertyBlock };
}

void MFrameShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info)
{
	for (const auto& pDecorator : m_vPropertyUpdateDecorator)
	{
		pDecorator->Update(info);
	}
}

void MFrameShaderPropertyBlock::RegisterPropertyDecorator(const std::shared_ptr<IShaderPropertyUpdateDecorator>& pDecorator)
{
	m_vPropertyUpdateDecorator.push_back(pDecorator);

	if (m_pShaderPropertyBlock)
	{
		pDecorator->BindMaterial(m_pShaderPropertyBlock);
	}
}

void MFrameShaderPropertyBlock::SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture)
{
	if (m_pBrdfMapTextureParam && m_pBrdfMapTextureParam->GetTexture() != pTexture)
	{
		m_pBrdfMapTextureParam->SetTexture(pTexture);
	}
}

void MFramePropertyDecorator::BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock)
{
	m_pFrameParam = pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_FRAME_DATA);
	MORTY_ASSERT(m_pFrameParam);
}

void MFramePropertyDecorator::Update(const MRenderInfo& info)
{
	if (!m_pFrameParam)
	{
		return;
	}

	MVariantStruct& cFrameStruct = m_pFrameParam->var.GetValue<MVariantStruct>();

	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_VIEW_MATRIX, info.m4CameraTransform.Inverse());
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_PROJ_MATRIX, info.m4CameraInverseProjection);
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_INV_CAMERA_PROJ_MATRIX, info.m4CameraInverseProjection.Inverse());

	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_POSITION, info.m4CameraTransform.GetTranslation());
	//FIXME no uniform scale.
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_CAMERA_DIRECTION, info.m4CameraTransform * Vector3(0, 0, 1));


	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_VIEWPORT_SIZE, Vector2(info.f2ViewportSize));
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_VIEWPORT_SIZE_INV, Vector2(1.0f / info.f2ViewportSize.x, 1.0f / info.f2ViewportSize.y));
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_Z_NEAR_FAR, info.f2CameraNearFar);

	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_TIME_DELTA, info.fDelta);
	cFrameStruct.SetVariant(MShaderPropertyName::FRAME_GAME_TIME, info.fGameTime);

	m_pFrameParam->SetDirty();
}

void MLightPropertyDecorator::BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock)
{
	MORTY_ASSERT(m_pLightParam = pShaderPropertyBlock->FindConstantParam(MShaderPropertyName::CBUFFER_LIGHT_DATA));
	MORTY_ASSERT(m_pDiffuseMapTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_IRRADIANCE_MAP));
	MORTY_ASSERT(m_pSpecularMapTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_PREFILTER_MAP));
}

void MLightPropertyDecorator::Update(const MRenderInfo& info)
{
	if (!m_pLightParam)
	{
		return;
	}
	MVariantStruct& cLightStruct = m_pLightParam->var.GetValue<MVariantStruct>();

	if (info.pEnvDiffuseTexture && info.pEnvSpecularTexture)
	{
		m_pDiffuseMapTextureParam->SetTexture(info.pEnvDiffuseTexture);
		m_pSpecularMapTextureParam->SetTexture(info.pEnvSpecularTexture);

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

	const size_t nPointLightNum = std::min(size_t(MRenderGlobal::POINT_LIGHT_MAX_NUMBER), info.vPointLight.size());
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

	m_pLightParam->SetDirty();
}

void MAnimationPropertyDecorator::BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock)
{
	MORTY_ASSERT(m_pAnimationBonesParam = pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_MATRIX));
	MORTY_ASSERT(m_pAnimationOffsetParam = pShaderPropertyBlock->FindStorageParam(MShaderPropertyName::STORAGE_BONES_OFFSET));

}

void MAnimationPropertyDecorator::Update(const MRenderInfo& info)
{
	const auto pScene = info.pScene;
	MORTY_ASSERT(pScene);

	if (const auto pAnimationManager = pScene->GetManager<MAnimationManager>())
	{
		const auto bufferData = pAnimationManager->GetAnimationBuffer();
		m_pAnimationBonesParam->pBuffer = bufferData.pBonesBuffer;
		m_pAnimationOffsetParam->pBuffer = bufferData.pOffsetBuffer;

		m_pAnimationBonesParam->SetDirty();
		m_pAnimationOffsetParam->SetDirty();
	}

}
