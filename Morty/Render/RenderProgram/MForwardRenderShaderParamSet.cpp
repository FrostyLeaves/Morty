#include "MForwardRenderShaderParamSet.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MShader.h"
#include "MViewport.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MSkyBoxComponent.h"
#include "MSpotLightComponent.h"
#include "MPointLightComponent.h"
#include "MDirectionalLightComponent.h"

#include "MRenderSystem.h"

MForwardRenderShaderParamSet::MForwardRenderShaderParamSet()
	: MShaderParamSet(1)
	, m_pWorldMatrixParam(nullptr)
	, m_pWorldInfoParam(nullptr)
	, m_pLightInfoParam(nullptr)
	
	, m_pShadowTextureParam(nullptr)
	, m_pEnvironmentTextureParam(nullptr)

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
	worldMatrixSrt.AppendMVariant("U_matProj", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matCamProj", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matCamProjInv", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matLightProj", Matrix4());

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_pWorldInfoParam = new MShaderConstantParam();
	m_pWorldInfoParam->unSet = 1;
	m_pWorldInfoParam->unBinding = 1;
	m_pWorldInfoParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;
	
	MStruct worldInfoSrt;
	worldInfoSrt.AppendMVariant("U_f3DirectionLight", Vector3());
	worldInfoSrt.AppendMVariant("U_f3CameraPosition", Vector3());
	worldInfoSrt.AppendMVariant("U_f3CameraDirection", Vector3());
	worldInfoSrt.AppendMVariant("U_f2ViewportSize", Vector2());
	worldInfoSrt.AppendMVariant("U_matZNearFar", Vector2());
	worldInfoSrt.AppendMVariant("U_fDelta", float());
	worldInfoSrt.AppendMVariant("U_fGameTime", float());

	m_pWorldInfoParam->var = worldInfoSrt;

	m_pLightInfoParam = new MShaderConstantParam();
	m_pLightInfoParam->unSet = 1;
	m_pLightInfoParam->unBinding = 2;
	m_pLightInfoParam->eShaderType = (uint32_t)MEShaderType::EPixel | (uint32_t)MEShaderType::EVertex;

	m_pLightInfoParam->var = MStruct();
	MStruct& lightInfoSrt = *m_pLightInfoParam->var.GetStruct();
	
	MStruct dirLightSrt;
	dirLightSrt.AppendMVariant("f3Intensity", Vector3());
	
	lightInfoSrt.AppendMVariant("U_dirLight", dirLightSrt);

	MVariantArray pointLightArray;
	for (uint32_t i = 0; i < MRenderGlobal::POINT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct pointLight;
		
		pointLight.AppendMVariant("f3WorldPosition", Vector3());
		pointLight.AppendMVariant("f3Intensity", Vector3());

		pointLight.AppendMVariant("fConstant", float(0.0f));
		pointLight.AppendMVariant("fLinear", float(0.0f));
		pointLight.AppendMVariant("fQuadratic", float(0.0f));
		
		pointLightArray.AppendMVariant(pointLight);
	}

	lightInfoSrt.AppendMVariant("U_spotLights", pointLightArray);

	MVariantArray spotLightArray;
	for (uint32_t i = 0; i < MRenderGlobal::SPOT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct spotLight;
	
		spotLight.AppendMVariant("f3WorldPosition", Vector3());
		spotLight.AppendMVariant("fHalfInnerCutOff", float(0.0f));
		spotLight.AppendMVariant("f3Direction", Vector3());
		spotLight.AppendMVariant("fHalfOuterCutOff", float(0.0f));
		spotLight.AppendMVariant("f3Intensity", Vector3());

		spotLightArray.AppendMVariant(spotLight);
	}

	lightInfoSrt.AppendMVariant("U_pointLights", spotLightArray);

	lightInfoSrt.AppendMVariant("U_bDirectionLightEnabled", int(0));
	lightInfoSrt.AppendMVariant("U_nValidPointLightsNumber", int(0));
	lightInfoSrt.AppendMVariant("U_nValidSpotLightsNumber", int(0));
	lightInfoSrt.AppendMVariant("U_bEnvironmentMapEnabled", int(0));

	MShaderSampleParam* pLinearSampler = new MShaderSampleParam();
	pLinearSampler->eSamplerType = MESamplerType::ELinear;
	pLinearSampler->unSet = 1;
	pLinearSampler->unBinding = 3;

	MShaderSampleParam* pNearestSampler = new MShaderSampleParam();
	pNearestSampler->eSamplerType = MESamplerType::ENearest;
	pNearestSampler->unSet = 1;
	pNearestSampler->unBinding = 4;

	m_pShadowTextureParam = new MShaderTextureParam();
	m_pShadowTextureParam->unSet = 1;
	m_pShadowTextureParam->unBinding = 6;

	m_pEnvironmentTextureParam = new MShaderTextureParam();
	m_pEnvironmentTextureParam->eType = METextureType::ETextureCube;
	m_pEnvironmentTextureParam->unSet = 1;
	m_pEnvironmentTextureParam->unBinding = 7;
	
	m_vParams.push_back(m_pWorldMatrixParam);
	m_vParams.push_back(m_pWorldInfoParam);
	m_vParams.push_back(m_pLightInfoParam);

	m_vSamples.push_back(pLinearSampler);
	m_vSamples.push_back(pNearestSampler);

	m_vTextures.push_back(m_pShadowTextureParam);
	m_vTextures.push_back(m_pEnvironmentTextureParam);
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
		cStruct[0] = info.pViewport->GetProjection();
		cStruct[1] = info.pViewport->GetCameraInverseProjection();
		cStruct[2] = info.pViewport->GetCameraInverseProjection().Inverse();
		cStruct[3] = info.m4DirLightInvProj;

		m_pWorldMatrixParam->SetDirty();
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
					m_pEnvironmentTextureParam->SetTexture(pEnvTexture);
					m_pEnvironmentTextureParam->SetDirty();
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
			std::vector<MPointLightComponent>& vActivePointLights = pComponentGroup->m_vComponent;
			
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
	if (m_pShadowTextureParam->pTexture != pTexture)
	{
		m_pShadowTextureParam->pTexture = pTexture;
		m_pShadowTextureParam->SetDirty();
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
