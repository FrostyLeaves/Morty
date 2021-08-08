#include "MForwardRenderShaderParamSet.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MViewport.h"

#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MSpotLightComponent.h"
#include "MPointLightComponent.h"
#include "MDirectionalLightComponent.h"

#include "MRenderSystem.h"

MForwardRenderShaderParamSet::MForwardRenderShaderParamSet()
	: MShaderParamSet(1)
	, m_pWorldMatrixParam(nullptr)
	, m_pWorldInfoParam(nullptr)
	, m_pLightInfoParam(nullptr)
	
	, m_pDefaultSampleParam(nullptr)

	, m_pShadowTextureParam(nullptr)

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

	MStruct worldMatrixSrt;
	worldMatrixSrt.AppendMVariant("U_matCamProj", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matCamProjInv", Matrix4());
	worldMatrixSrt.AppendMVariant("U_matLightProj", Matrix4());

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_pWorldInfoParam = new MShaderConstantParam();
	m_pWorldInfoParam->unSet = 1;
	m_pWorldInfoParam->unBinding = 1;
	
	MStruct worldInfoSrt;
	worldInfoSrt.AppendMVariant("U_f3DirectionLight", Vector3());
	worldInfoSrt.AppendMVariant("U_f3CameraPosition", Vector3());
	worldInfoSrt.AppendMVariant("U_f2ViewportSize", Vector2());
	worldInfoSrt.AppendMVariant("U_matZNearFar", Vector2());
	worldInfoSrt.AppendMVariant("U_fDelta", float());
	worldInfoSrt.AppendMVariant("U_fGameTime", float());

	m_pWorldInfoParam->var = worldInfoSrt;

	m_pLightInfoParam = new MShaderConstantParam();
	m_pLightInfoParam->unSet = 1;
	m_pLightInfoParam->unBinding = 2;

	m_pLightInfoParam->var = MStruct();
	MStruct& lightInfoSrt = *m_pLightInfoParam->var.GetStruct();
	
	MStruct dirLightSrt;
	dirLightSrt.AppendMVariant("f3Diffuse", Vector3());
	dirLightSrt.AppendMVariant("f3Specular", Vector3());
	
	lightInfoSrt.AppendMVariant("U_dirLight", dirLightSrt);

	MVariantArray pointLightArray;
	for (uint32_t i = 0; i < MGlobal::POINT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct pointLight;
		
		pointLight.AppendMVariant("f3WorldPosition", Vector3());
		pointLight.AppendMVariant("f3Diffuse", Vector3());
		pointLight.AppendMVariant("f3Specular", Vector3());

		pointLight.AppendMVariant("fConstant", float(0.0f));
		pointLight.AppendMVariant("fLinear", float(0.0f));
		pointLight.AppendMVariant("fQuadratic", float(0.0f));
		
		pointLightArray.AppendMVariant(pointLight);
	}

	lightInfoSrt.AppendMVariant("U_spotLights", pointLightArray);

	MVariantArray spotLightArray;
	for (uint32_t i = 0; i < MGlobal::SPOT_LIGHT_MAX_NUMBER; ++i)
	{
		MStruct spotLight;
	
		spotLight.AppendMVariant("f3WorldPosition", Vector3());
		spotLight.AppendMVariant("fHalfInnerCutOff", float(0.0f));
		spotLight.AppendMVariant("f3Direction", Vector3());
		spotLight.AppendMVariant("fHalfOuterCutOff", float(0.0f));
		spotLight.AppendMVariant("f3Diffuse", Vector3());
		spotLight.AppendMVariant("f3Specular", Vector3());

		spotLightArray.AppendMVariant(spotLight);
	}

	lightInfoSrt.AppendMVariant("U_pointLights", spotLightArray);

	lightInfoSrt.AppendMVariant("U_bDirectionLightEnabled", int(0));
	lightInfoSrt.AppendMVariant("U_nValidPointLightsNumber", int(0));
	lightInfoSrt.AppendMVariant("U_nValidSpotLightsNumber", int(0));


	m_pDefaultSampleParam = new MShaderSampleParam();
	m_pDefaultSampleParam->unSet = 1;
	m_pDefaultSampleParam->unBinding = 3;

	m_pShadowTextureParam = new MShaderTextureParam();
	m_pShadowTextureParam->unSet = 1;
	m_pShadowTextureParam->unBinding = 6;
	

	m_vParams.push_back(m_pWorldMatrixParam);
	m_vParams.push_back(m_pWorldInfoParam);
	m_vParams.push_back(m_pLightInfoParam);


	m_vSamples.push_back(m_pDefaultSampleParam);


	m_vTextures.push_back(m_pShadowTextureParam);
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

	pViewport->LockMatrix();

	if (m_pWorldMatrixParam)
	{
		MStruct& cStruct = *m_pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pViewport->GetCameraInverseProjection();
		cStruct[1] = info.pViewport->GetCameraInverseProjection().Inverse();
		cStruct[2] = info.m4DirLightInvProj;

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
			}
		}

		(*m_pWorldInfoParam->var.GetStruct())[2] = info.pViewport->GetSize();

		if (info.pCameraEntity)
		{
			if (MCameraComponent* pCameraComponent = info.pCameraEntity->GetComponent<MCameraComponent>())
			{
				(*m_pWorldInfoParam->var.GetStruct())[3] = pCameraComponent->GetZNearFar();
			}
		}

		(*m_pWorldInfoParam->var.GetStruct())[4] = info.fDelta;

		(*m_pWorldInfoParam->var.GetStruct())[5] = info.fGameTime;

		m_pWorldInfoParam->SetDirty();
	}

	if (MShaderConstantParam* pLightParam = m_pLightInfoParam)
	{
		MVariant& varDirLightEnable = (*pLightParam->var.GetStruct())[3];
		if (info.pDirectionalLightEntity)
		{
			varDirLightEnable = 500;
			MVariant& varDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *varDirectionLight.GetStruct();
				{
					if (MDirectionalLightComponent* pLightComponent = info.pDirectionalLightEntity->GetComponent<MDirectionalLightComponent>())
					{
						cLightStruct[0] = pLightComponent->GetColor().ToVector3() * pLightComponent->GetLightIntensity();
						cLightStruct[1] = pLightComponent->GetColor().ToVector3() * pLightComponent->GetLightIntensity();
					}
				}
			}
		}
		else
		{
			varDirLightEnable = false;
		}

// 		MVariant& varPointLights = (*pLightParam->var.GetStruct())[1];
// 		MVariant& varValidPointLights = (*pLightParam->var.GetStruct())[4];
// 		{
// 			std::vector<MPointLightComponent*> vActivePointLights(MGlobal::POINT_LIGHT_MAX_NUMBER);
// 			info.pScene->FindActivePointLights(info.pCameraSceneComponent->GetWorldPosition(), vActivePointLights);
// 			varValidPointLights = 0;
// 
// 			MVariantArray& vPointLights = *varPointLights.GetArray();
// 			for (uint32_t i = 0; i < vPointLights.GetMemberCount(); ++i)
// 			{
// 				if (MPointLightComponent* pPointLightComponent = vActivePointLights[i])
// 				{
// 					MNode* pNode = pPointLightComponent->GetOwnerNode();
// 					MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
// 					
// 					MStruct& cPointLight = *vPointLights[i].GetStruct();
// 					cPointLight[0] = pSceneComponent->GetWorldPosition();
// 					cPointLight[1] = pPointLightComponent->GetColor().ToVector3() * pPointLightComponent->GetLightIntensity();
// 					cPointLight[2] = pPointLightComponent->GetColor().ToVector3() * pPointLightComponent->GetLightIntensity();
// 
// 					cPointLight[3] = pPointLightComponent->GetConstant();
// 					cPointLight[4] = pPointLightComponent->GetLinear();
// 					cPointLight[5] = pPointLightComponent->GetQuadratic();
// 
// 					varValidPointLights = (int)i + 1;
// 				}
// 				else break;
// 			}
// 		}
// 
// 		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
// 		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
// 		{
// 			std::vector<MSpotLightComponent*> vActiveSpotLights(MGlobal::SPOT_LIGHT_MAX_NUMBER);
// 			info.pScene->FindActiveSpotLights(info.pCameraSceneComponent->GetWorldPosition(), vActiveSpotLights);
// 			varValidSpotLights = 0;
// 
// 			MVariantArray& vSpotLights = *varSpotLights.GetArray();
// 			for (uint32_t i = 0; i < vSpotLights.GetMemberCount(); ++i)
// 			{
// 				if (MSpotLightComponent* pSpotLightComponent = vActiveSpotLights[i])
// 				{
// 					MNode* pNode = pSpotLightComponent->GetOwnerNode();
// 					MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
// 
// 					Vector3 f3SpotDirection = pSpotLightComponent->GetWorldDirection();
// 					f3SpotDirection.Normalize();
// 					MStruct& cSpotLight = *vSpotLights[i].GetStruct();
// 					cSpotLight[0] = pSceneComponent->GetWorldPosition();
// 					cSpotLight[1] = pSpotLightComponent->GetInnerCutOffRadius();
// 					cSpotLight[2] = f3SpotDirection;
// 					cSpotLight[3] = pSpotLightComponent->GetOuterCutOffRadius();
// 					cSpotLight[4] = pSpotLightComponent->GetColor().ToVector3() * pSpotLightComponent->GetLightIntensity();
// 					cSpotLight[5] = pSpotLightComponent->GetColor().ToVector3() * pSpotLightComponent->GetLightIntensity();
// 
// 					varValidSpotLights = (int)i + 1;
// 				}
// 				else break;
// 			}
// 		}

		pLightParam->SetDirty();
	}


	pViewport->UnlockMatrix();
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
