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

#include "System/MRenderSystem.h"

MForwardRenderShaderParamSet::MForwardRenderShaderParamSet()
	: MShaderParamSet(1)
	, m_pWorldMatrixParam(nullptr)
	, m_pWorldInfoParam(nullptr)
	, m_pLightInfoParam(nullptr)
	, m_pShadowInfoParam(nullptr)
	
	, m_pShadowTextureParam(nullptr)
	, m_pDiffuseMapTextureParam(nullptr)
	, m_pSpecularMapTextureParam(nullptr)
	, m_pBrdfMapTextureParam(nullptr)

{
	
}

MForwardRenderShaderParamSet::~MForwardRenderShaderParamSet()
{
}

void MForwardRenderShaderParamSet::InitializeShaderParamSet(MEngine* pEngine)
{
	m_pWorldMatrixParam = new MShaderConstantParam();
	m_pWorldMatrixParam->unSet = 1;
	m_pWorldMatrixParam->unBinding = 0;
	m_pWorldMatrixParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;

	MStruct worldMatrixSrt;
	worldMatrixSrt.SetValue("U_matView", Matrix4());
	worldMatrixSrt.SetValue("U_matCamProj", Matrix4());
	worldMatrixSrt.SetValue("U_matCamProjInv", Matrix4());

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_pWorldInfoParam = new MShaderConstantParam();
	m_pWorldInfoParam->unSet = 1;
	m_pWorldInfoParam->unBinding = 1;
	m_pWorldInfoParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;
	m_pWorldInfoParam->var = MStruct();

	MStruct& worldInfoSrt = *m_pWorldInfoParam->var.GetStruct();
	worldInfoSrt.SetValue("U_f3DirectionLight", Vector3());
	worldInfoSrt.SetValue("U_f3CameraPosition", Vector3());
	worldInfoSrt.SetValue("U_f3CameraDirection", Vector3());
	worldInfoSrt.SetValue("U_f2ViewportSize", Vector2());
	worldInfoSrt.SetValue("U_matZNearFar", Vector2());
	worldInfoSrt.SetValue("U_fDelta", float());
	worldInfoSrt.SetValue("U_fGameTime", float());


	m_pLightInfoParam = new MShaderConstantParam();
	m_pLightInfoParam->unSet = 1;
	m_pLightInfoParam->unBinding = 2;
	m_pLightInfoParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;
	m_pLightInfoParam->var = MStruct();

	MStruct& lightInfoSrt = *m_pLightInfoParam->var.GetStruct();
	
	MStruct dirLightSrt;
	dirLightSrt.SetValue("f3Intensity", Vector3());
	
	lightInfoSrt.SetValue("U_dirLight", dirLightSrt);

	MVariantArray pointLightArray;
	for (uint32_t i = 0; i < MRenderGlobal::POINT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct pointLight;
		
		pointLight.SetValue("f3WorldPosition", Vector3());
		pointLight.SetValue("f3Intensity", Vector3());

		pointLight.SetValue("fConstant", float(0.0f));
		pointLight.SetValue("fLinear", float(0.0f));
		pointLight.SetValue("fQuadratic", float(0.0f));
		
		pointLightArray.AppendValue(pointLight);
	}

	lightInfoSrt.SetValue("U_spotLights", pointLightArray);

	MVariantArray spotLightArray;
	for (uint32_t i = 0; i < MRenderGlobal::SPOT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct spotLight;
	
		spotLight.SetValue("f3WorldPosition", Vector3());
		spotLight.SetValue("fHalfInnerCutOff", float(0.0f));
		spotLight.SetValue("f3Direction", Vector3());
		spotLight.SetValue("fHalfOuterCutOff", float(0.0f));
		spotLight.SetValue("f3Intensity", Vector3());

		spotLightArray.AppendValue(spotLight);
	}

	lightInfoSrt.SetValue("U_pointLights", spotLightArray);

	lightInfoSrt.SetValue("U_bDirectionLightEnabled", int(0));
	lightInfoSrt.SetValue("U_nValidPointLightsNumber", int(0));
	lightInfoSrt.SetValue("U_nValidSpotLightsNumber", int(0));
	lightInfoSrt.SetValue("U_bEnvironmentMapEnabled", int(0));


	m_pShadowInfoParam = new MShaderConstantParam();
	m_pShadowInfoParam->unSet = 1;
	m_pShadowInfoParam->unBinding = 3;
	m_pShadowInfoParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;
	m_pShadowInfoParam->var = MStruct();

	MStruct& shadowInfoSrt = *m_pShadowInfoParam->var.GetStruct();

	MVariantArray matLightProjArray;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		matLightProjArray.AppendValue<Matrix4>();
	}
	shadowInfoSrt.SetValue("U_matLightProj", matLightProjArray);

	MVariantArray matCascadeSplitArray;
	for (size_t nCascadedIdx = 0; nCascadedIdx < MRenderGlobal::CASCADED_SHADOW_MAP_NUM; ++nCascadedIdx)
	{
		matCascadeSplitArray.AppendValue<float>();
	}
	shadowInfoSrt.SetValue("U_vCascadeSplits", matCascadeSplitArray);


	MShaderSampleParam* pLinearSampler = new MShaderSampleParam();
	pLinearSampler->eSamplerType = MESamplerType::ELinear;
	pLinearSampler->unSet = 1;
	pLinearSampler->unBinding = 4;

	MShaderSampleParam* pNearestSampler = new MShaderSampleParam();
	pNearestSampler->eSamplerType = MESamplerType::ENearest;
	pNearestSampler->unSet = 1;
	pNearestSampler->unBinding = 5;

	m_pShadowTextureParam = new MShaderTextureParam();
	m_pShadowTextureParam->unSet = 1;
	m_pShadowTextureParam->unBinding = 6;
	m_pShadowTextureParam->eType = METextureType::ETexture2DArray;

	m_pDiffuseMapTextureParam = new MShaderTextureParam();
	m_pDiffuseMapTextureParam->eType = METextureType::ETextureCube;
	m_pDiffuseMapTextureParam->unSet = 1;
	m_pDiffuseMapTextureParam->unBinding = 7;

	m_pSpecularMapTextureParam = new MShaderTextureParam();
	m_pSpecularMapTextureParam->eType = METextureType::ETextureCube;
	m_pSpecularMapTextureParam->unSet = 1;
	m_pSpecularMapTextureParam->unBinding = 8;

	m_pBrdfMapTextureParam = new MShaderTextureParam();
	m_pBrdfMapTextureParam->eType = METextureType::ETexture2D;
	m_pBrdfMapTextureParam->unSet = 1;
	m_pBrdfMapTextureParam->unBinding = 9;
	
	m_vParams.push_back(m_pWorldMatrixParam);
	m_vParams.push_back(m_pWorldInfoParam);
	m_vParams.push_back(m_pLightInfoParam);
	m_vParams.push_back(m_pShadowInfoParam);

	m_vSamples.push_back(pLinearSampler);
	m_vSamples.push_back(pNearestSampler);

	m_vTextures.push_back(m_pShadowTextureParam);
	m_vTextures.push_back(m_pDiffuseMapTextureParam);
	m_vTextures.push_back(m_pSpecularMapTextureParam);
	m_vTextures.push_back(m_pBrdfMapTextureParam);
}

void MForwardRenderShaderParamSet::ReleaseShaderParamSet(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	DestroyBuffer(pRenderSystem->GetDevice());
}

void MForwardRenderShaderParamSet::UpdateShaderSharedParams(MRenderInfo& info)
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

	if (MShaderConstantParam* pLightParam = m_pLightInfoParam)
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

void MForwardRenderShaderParamSet::SetShadowMapTexture(MTexture* pTexture)
{
	if (m_pShadowTextureParam->GetTexture() != pTexture)
	{
		m_pShadowTextureParam->SetTexture(pTexture);
	}
}

void MForwardRenderShaderParamSet::SetBrdfMapTexture(MTexture* pTexture)
{
	if (m_pBrdfMapTextureParam->GetTexture() != pTexture)
	{
		m_pBrdfMapTextureParam->SetTexture(pTexture);
	}
}

MForwardRenderTransparentShaderParamSet::MForwardRenderTransparentShaderParamSet()
	: MForwardRenderShaderParamSet()
	, m_pTransparentFrontTextureParam(nullptr)
	, m_pTransparentBackTextureParam(nullptr)
{

}

MForwardRenderTransparentShaderParamSet::~MForwardRenderTransparentShaderParamSet()
{

}

void MForwardRenderTransparentShaderParamSet::InitializeShaderParamSet(MEngine* pEngine)
{
	MForwardRenderShaderParamSet::InitializeShaderParamSet(pEngine);

	m_pTransparentFrontTextureParam = new MShaderSubpasssInputParam();
	m_pTransparentFrontTextureParam->unSet = 1;
	m_pTransparentFrontTextureParam->unBinding = 7;
	m_pTransparentBackTextureParam = new MShaderSubpasssInputParam();
	m_pTransparentBackTextureParam->unSet = 1;
	m_pTransparentBackTextureParam->unBinding = 8;

	m_vTextures.push_back(m_pTransparentFrontTextureParam);
	m_vTextures.push_back(m_pTransparentBackTextureParam);
}

void MForwardRenderTransparentShaderParamSet::ReleaseShaderParamSet(MEngine* pEngine)
{
	MForwardRenderShaderParamSet::ReleaseShaderParamSet(pEngine);
}
