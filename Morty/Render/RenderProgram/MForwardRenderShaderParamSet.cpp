#include "MForwardRenderShaderParamSet.h"

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

#include "System/MRenderSystem.h"

MForwardRenderShaderPropertyBlock::MForwardRenderShaderPropertyBlock()
	: m_pWorldMatrixParam(nullptr)
	, m_pWorldInfoParam(nullptr)
	, m_pLightInfoParam(nullptr)
	, m_pShadowInfoParam(nullptr)
	, m_pShadowTextureParam(nullptr)
	, m_pDiffuseMapTextureParam(nullptr)
	, m_pSpecularMapTextureParam(nullptr)
	, m_pBrdfMapTextureParam(nullptr)

{
	
}

MForwardRenderShaderPropertyBlock::~MForwardRenderShaderPropertyBlock()
{
}

void MForwardRenderShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pShaderPropertyBlock = pMaterial->GetFrameParamSet()->Clone();

	MORTY_ASSERT(m_pWorldMatrixParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneMatrix"));
	MORTY_ASSERT(m_pWorldInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbSceneInformation"));
	MORTY_ASSERT(m_pLightInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbLightInformation"));
	MORTY_ASSERT(m_pShadowInfoParam = m_pShaderPropertyBlock->FindConstantParam("cbShadowInformation"));

	MORTY_ASSERT(m_pShadowTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texShadowMap"));
	MORTY_ASSERT(m_pDiffuseMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texIrradianceMap"));
	MORTY_ASSERT(m_pSpecularMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texPrefilterMap"));
	MORTY_ASSERT(m_pBrdfMapTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texBrdfLUT"));
}

void MForwardRenderShaderPropertyBlock::ReleaseShaderParamSet(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_pShaderPropertyBlock->DestroyBuffer(pRenderSystem->GetDevice());
	m_pShaderPropertyBlock = nullptr;
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

 	if (!info.pSkyBoxEntity)
 	{
 		info.pSkyBoxEntity = pScene->FindFirstEntityByComponent<MSkyBoxComponent>();
 	}

	pViewport->LockMatrix();

	if (m_pWorldMatrixParam)
	{
		MStruct& cStruct = *m_pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pCameraEntity->GetComponent<MSceneComponent>()->GetWorldTransform().Inverse();
		cStruct[1] = info.pViewport->GetCameraInverseProjection();
 		cStruct[2] = info.pViewport->GetCameraInverseProjection().Inverse();

		m_pWorldMatrixParam->SetDirty();
	}

	if (m_pShadowInfoParam)
	{
		MStruct& cStruct = *m_pShadowInfoParam->var.GetStruct();
		if (MVariantArray* pDirLightInvProjArray = cStruct[0].GetArray())
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				(*pDirLightInvProjArray)[nCascadedIdx] = info.cCascadedShadow[nCascadedIdx].m4DirLightInvProj;
			}
		}

		if (MVariantArray* pSplitDepthArray = cStruct[1].GetArray())
		{
			for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
			{
				(*pSplitDepthArray)[nCascadedIdx] = info.cCascadedShadow[nCascadedIdx].fSplitDepth;
			}
		}

		m_pShadowInfoParam->SetDirty();
	}

	if (m_pWorldInfoParam)
	{
		if (info.pDirectionalLightEntity)
		{
			if (MSceneComponent* pSceneComponent = info.pDirectionalLightEntity->GetComponent<MSceneComponent>())
			{
				(*m_pWorldInfoParam->var.GetStruct())[0] = pSceneComponent->GetForward();
			}
		}

		if (info.pCameraEntity)
		{
			if (MSceneComponent* pSceneComponent = info.pCameraEntity->GetComponent<MSceneComponent>())
			{
				(*m_pWorldInfoParam->var.GetStruct())[1] = pSceneComponent->GetWorldPosition();
				(*m_pWorldInfoParam->var.GetStruct())[2] = pSceneComponent->GetWorldForward();
			}
		}

		(*m_pWorldInfoParam->var.GetStruct())[3] = info.pViewport->GetSize();

		if (info.pCameraEntity)
		{
			if (MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>())
			{
				(*m_pWorldInfoParam->var.GetStruct())[4] = pCameraComponent->GetZNearFar();
			}
		}

		(*m_pWorldInfoParam->var.GetStruct())[5] = info.fDelta;

		(*m_pWorldInfoParam->var.GetStruct())[6] = info.fGameTime;

		m_pWorldInfoParam->SetDirty();
	}

	if (const std::shared_ptr<MShaderConstantParam>& pLightParam = m_pLightInfoParam)
	{
		if (info.pSkyBoxEntity)
		{
			if (MSkyBoxComponent* pSkyBoxComponent = info.pSkyBoxEntity->GetComponent<MSkyBoxComponent>())
			{
				if (MTexture* pEnvTexture = pSkyBoxComponent->GetDiffuseTexture())
				{
					MVariant& varEnvMapEnable = (*pLightParam->var.GetStruct())[6];
					varEnvMapEnable = true;
					m_pDiffuseMapTextureParam->SetTexture(pEnvTexture);
					m_pDiffuseMapTextureParam->SetDirty();
				}
				if (MTexture* pEnvTexture = pSkyBoxComponent->GetSpecularTexture())
				{
					m_pSpecularMapTextureParam->SetTexture(pEnvTexture);
					m_pSpecularMapTextureParam->SetDirty();
				}
			}
		}

		MVariant& varDirLightEnable = (*pLightParam->var.GetStruct())[3];
		if (info.pDirectionalLightEntity)
		{
			varDirLightEnable = true;
			MVariant& varDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *varDirectionLight.GetStruct();
				{
					if (MDirectionalLightComponent* pLightComponent = info.pDirectionalLightEntity->GetComponent<MDirectionalLightComponent>())
					{
						cLightStruct[0] = pLightComponent->GetColor().ToVector3() * pLightComponent->GetLightIntensity();
					}
				}
			}
		}
		else
		{
			varDirLightEnable = false;
		}

   		MVariant& varPointLights = (*pLightParam->var.GetStruct())[1];
   		MVariant& varValidPointLights = (*pLightParam->var.GetStruct())[4];
   		{
			MComponentGroup<MPointLightComponent>* pComponentGroup = pScene->FindComponents<MPointLightComponent>();
			auto& vActivePointLights = pComponentGroup->m_vComponents;
			
   			//info.pScene->FindActivePointLights(info.pCameraSceneComponent->GetWorldPosition(), vActivePointLights);
			int nValidPointLights = 0;
 
   			MVariantArray& vPointLights = *varPointLights.GetArray();
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
				
				MStruct& cPointLight = *vPointLights[nValidPointLights].GetStruct();
				cPointLight[0] = pSceneComponent->GetWorldPosition();
				cPointLight[1] = pPointLightComponent->GetColor().ToVector3() * pPointLightComponent->GetLightIntensity();

				cPointLight[2] = pPointLightComponent->GetConstant();
				cPointLight[3] = pPointLightComponent->GetLinear();
				cPointLight[4] = pPointLightComponent->GetQuadratic();

				++nValidPointLights;

				if (nValidPointLights >= MRenderGlobal::POINT_LIGHT_MAX_NUMBER)
					break;
   			}

			varValidPointLights = nValidPointLights;
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


	pViewport->UnlockMatrix();
}

void MForwardRenderShaderPropertyBlock::SetShadowMapTexture(MTexture* pTexture)
{
	if (m_pShadowTextureParam->GetTexture() != pTexture)
	{
		m_pShadowTextureParam->SetTexture(pTexture);
	}
}

void MForwardRenderShaderPropertyBlock::SetBrdfMapTexture(MTexture* pTexture)
{
	if (m_pBrdfMapTextureParam->GetTexture() != pTexture)
	{
		m_pBrdfMapTextureParam->SetTexture(pTexture);
	}
}

MForwardRenderTransparentShaderPropertyBlock::MForwardRenderTransparentShaderPropertyBlock()
	: MForwardRenderShaderPropertyBlock()
	, m_pTransparentFrontTextureParam(nullptr)
	, m_pTransparentBackTextureParam(nullptr)
{

}

MForwardRenderTransparentShaderPropertyBlock::~MForwardRenderTransparentShaderPropertyBlock()
{

}

void MForwardRenderTransparentShaderPropertyBlock::BindMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	MForwardRenderShaderPropertyBlock::BindMaterial(pMaterial);

	MORTY_ASSERT(m_pTransparentFrontTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texSubpassInput0"));
	MORTY_ASSERT(m_pTransparentBackTextureParam = m_pShaderPropertyBlock->FindTextureParam("u_texSubpassInput1"));
}

void MForwardRenderTransparentShaderPropertyBlock::ReleaseShaderParamSet(MEngine* pEngine)
{
	MForwardRenderShaderPropertyBlock::ReleaseShaderParamSet(pEngine);
}
