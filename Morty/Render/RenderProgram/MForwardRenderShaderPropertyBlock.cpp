#include "MForwardRenderShaderPropertyBlock.h"

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

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

void MForwardRenderShaderPropertyBlock::Initialize(MEngine* pEngine)
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

void MForwardRenderShaderPropertyBlock::Release(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;

	m_pMaterial = nullptr;
}


std::shared_ptr<MMaterial> MForwardRenderShaderPropertyBlock::LoadMaterial(MEngine* pEngine) const
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Deferred/model_gbuffer.mvs");
	std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Deferred/model_gbuffer.mps");
	auto pMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	pMaterial->SetCullMode(MECullMode::ECullBack);
	pMaterial->LoadVertexShader(forwardVS);
	pMaterial->LoadPixelShader(forwardPS);

	return pMaterial;
}

void MForwardRenderShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pShaderPropertyBlock = pMaterial->GetFramePropertyBlock()->Clone();

	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneMatrix"));
	MORTY_ASSERT(m_pWorldInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneInformation"));
	MORTY_ASSERT(m_pLightInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbLightInformation"));
	MORTY_ASSERT(m_pShadowInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbShadowInformation"));

	MORTY_ASSERT(m_pShadowTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texShadowMap"));
	MORTY_ASSERT(m_pDiffuseMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texIrradianceMap"));
	MORTY_ASSERT(m_pSpecularMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texPrefilterMap"));
	MORTY_ASSERT(m_pBrdfMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texBrdfLUT"));
}

std::shared_ptr<MShaderPropertyBlock> MForwardRenderShaderPropertyBlock::GetPropertyBlock() const
{
	return { m_pShaderPropertyBlock };
}

void MForwardRenderShaderPropertyBlock::UpdateShaderSharedParams(MRenderInfo& info)
{
	MViewport* pViewport = info.pViewport;
	if (!pViewport) return;

	MScene* pScene = pViewport->GetScene();
	if (!pScene) return;

	info.pCameraEntity = pScene->FindFirstEntityByComponent<MCameraComponent>();

	if (!info.pDirectionalLightEntity)
	{
		info.pDirectionalLightEntity = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	}

	if (info.pDirectionalLightEntity)
	{
		if (const auto pLightComponent = info.pDirectionalLightEntity->GetComponent<MDirectionalLightComponent>())
		{
			if (!pLightComponent->GetLightEnable())
			{
				info.pDirectionalLightEntity = nullptr;
			}
		}
	}

 	if (!info.pSkyBoxEntity)
 	{
 		info.pSkyBoxEntity = pScene->FindFirstEntityByComponent<MSkyBoxComponent>();
 	}

	if (m_pWorldMatrixParam)
	{
		MVariantStruct& cStruct = m_pWorldMatrixParam->var.GetValue<MVariantStruct>();
		cStruct.SetVariant("u_matView",info.pCameraEntity->GetComponent<MSceneComponent>()->GetWorldTransform().Inverse());
		cStruct.SetVariant("u_matCamProj", info.m4CameraInverseProjection);
 		cStruct.SetVariant("u_matCamProjInv", info.m4CameraInverseProjection.Inverse());

		m_pWorldMatrixParam->SetDirty();
	}

	if (m_pShadowInfoParam)
	{
		MVariantStruct& cStruct = m_pShadowInfoParam->var.GetValue<MVariantStruct>();
		MVariantArray& cDirLightInvProjArray = cStruct.GetVariant<MVariantArray>("u_vLightProjectionMatrix");
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				cDirLightInvProjArray.SetVariant(nCascadedIdx, info.shadowRenderInfo[nCascadedIdx].m4DirLightInvProj);
			}
		}

		MVariantArray& cSplitDepthArray = cStruct.GetVariant<MVariantArray>("u_vCascadeSplits");
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				cSplitDepthArray.SetVariant(nCascadedIdx, info.shadowRenderInfo[nCascadedIdx].fSplitRange);
			}
		}

		m_pShadowInfoParam->SetDirty();
	}

	if (m_pWorldInfoParam)
	{
		MVariantStruct& cWorldInfo = m_pWorldInfoParam->var.GetValue<MVariantStruct>();
		

		if (info.pCameraEntity)
		{
			if (MSceneComponent* pSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>())
			{
				cWorldInfo.SetVariant("u_f3CameraPosition", pSceneComponent->GetWorldPosition());
				cWorldInfo.SetVariant("u_f3CameraDirection", pSceneComponent->GetWorldForward());
			}
		}

		cWorldInfo.SetVariant("u_f2ViewportSize", info.pViewport->GetSize());

		if (info.pCameraEntity)
		{
			if (MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>())
			{
				cWorldInfo.SetVariant("u_matZNearFar", pCameraComponent->GetZNearFar());
			}
		}

		cWorldInfo.SetVariant("u_fDelta", info.fDelta);

		cWorldInfo.SetVariant("u_fGameTime", info.fGameTime);

		m_pWorldInfoParam->SetDirty();
	}

	if (const std::shared_ptr<MShaderConstantParam>& pLightParam = m_pLightInfoParam)
	{
		MVariantStruct& cLightStruct = pLightParam->var.GetValue<MVariantStruct>();

		if (info.pSkyBoxEntity)
		{
			if (MSkyBoxComponent* pSkyBoxComponent = info.pSkyBoxEntity->GetComponent<MSkyBoxComponent>())
			{
				if (std::shared_ptr<MTexture> pEnvTexture = pSkyBoxComponent->GetDiffuseTexture())
				{
					cLightStruct.SetVariant("u_bEnvironmentMapEnabled", 1);
					m_pDiffuseMapTextureParam->SetTexture(pEnvTexture);
					m_pDiffuseMapTextureParam->SetDirty();
				}
				if (std::shared_ptr<MTexture> pEnvTexture = pSkyBoxComponent->GetSpecularTexture())
				{
					m_pSpecularMapTextureParam->SetTexture(pEnvTexture);
					m_pSpecularMapTextureParam->SetDirty();
				}
			}
		}

		if (info.pDirectionalLightEntity)
		{
			cLightStruct.SetVariant("u_bDirectionLightEnabled", 1);
			{
				MVariantStruct& cDirectionLightStruct = cLightStruct.GetVariant<MVariantStruct>("u_xDirectionalLight");
				{
					if (MSceneComponent* pSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>())
					{
						cDirectionLightStruct.SetVariant("f3LightDir", pSceneComponent->GetForward());
					}
					if (MDirectionalLightComponent* pLightComponent = info.pDirectionalLightEntity->GetComponent<MDirectionalLightComponent>())
					{
						cDirectionLightStruct.SetVariant("f3Intensity",pLightComponent->GetColor().ToVector3() * pLightComponent->GetLightIntensity());
						cDirectionLightStruct.SetVariant("fLightSize", pLightComponent->GetLightSize());
					}
				}
			}
		}
		else
		{
			cLightStruct.SetVariant("u_bDirectionLightEnabled", 0);
		}

   		{
			MComponentGroup<MPointLightComponent>* pComponentGroup = pScene->FindComponents<MPointLightComponent>();
			auto& vActivePointLights = pComponentGroup->m_vComponents;
			
   			//info.pScene->FindActivePointLights(info.pCameraSceneComponent->GetWorldPosition(), vActivePointLights);
			int nValidPointLights = 0;
 
   			MVariantArray& vPointLights = cLightStruct.GetVariant<MVariantArray>("u_vPointLights");
   			for (MPointLightComponent& lightComponent : vActivePointLights)
   			{
				if (!lightComponent.IsValid())
					break;

				MPointLightComponent* pPointLightComponent = &lightComponent;

				MEntity* pEntity = pPointLightComponent->GetEntity();
				if (!pEntity)
					break;

				MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
				if (!pSceneComponent)
					break;
				
				MVariantStruct cPointLight = vPointLights.GetVariant<MVariantStruct>(nValidPointLights);
				cPointLight.SetVariant("f3WorldPosition", pSceneComponent->GetWorldPosition());
				cPointLight.SetVariant("f3Intensity", pPointLightComponent->GetColor().ToVector3()* pPointLightComponent->GetLightIntensity());

				cPointLight.SetVariant("fConstant",pPointLightComponent->GetConstant());
				cPointLight.SetVariant("fLinear", pPointLightComponent->GetLinear());
				cPointLight.SetVariant("fQuadratic",pPointLightComponent->GetQuadratic());

				++nValidPointLights;

				if (nValidPointLights >= MRenderGlobal::POINT_LIGHT_MAX_NUMBER)
					break;
   			}

			cLightStruct.SetVariant("u_nValidPointLightsNumber", nValidPointLights);
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
		pLightParam->SetDirty();
	}

}

void MForwardRenderShaderPropertyBlock::SetShadowMapTexture(std::shared_ptr<MTexture> pTexture)
{
	if (m_pShadowTextureParam && m_pShadowTextureParam->GetTexture() != pTexture)
	{
		m_pShadowTextureParam->SetTexture(pTexture);
	}
}

void MForwardRenderShaderPropertyBlock::SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture)
{
	if (m_pBrdfMapTextureParam && m_pBrdfMapTextureParam->GetTexture() != pTexture)
	{
		m_pBrdfMapTextureParam->SetTexture(pTexture);
	}
}

void MForwardRenderTransparentShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MForwardRenderShaderPropertyBlock::BindMaterial(pMaterial);

	MORTY_ASSERT(m_pTransparentFrontTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texSubpassInput0"));
	MORTY_ASSERT(m_pTransparentBackTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texSubpassInput1"));
}
