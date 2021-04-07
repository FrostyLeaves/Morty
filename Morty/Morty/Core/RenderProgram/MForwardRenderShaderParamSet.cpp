#include "MForwardRenderShaderParamSet.h"

#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"
#include "MPainter.h"
#include "MViewport.h"
#include "MIRenderer.h"

#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

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
	worldMatrixSrt.AppendMVariant("U_matLightProj", Matrix4());

	m_pWorldMatrixParam->var = worldMatrixSrt;

	m_pWorldInfoParam = new MShaderConstantParam();
	m_pWorldInfoParam->unSet = 1;
	m_pWorldInfoParam->unBinding = 1;
	
	MStruct worldInfoSrt;
	worldInfoSrt.AppendMVariant("U_f3DirectionLight", Vector3());
	worldInfoSrt.AppendMVariant("U_f3CameraPosition", Vector3());
	worldInfoSrt.AppendMVariant("U_f2ViewportSize", Vector2());
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
	for (uint32_t i = 0; i < MGlobal::MPOINT_LIGHT_MAX_NUMBER; ++i)
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
	for (uint32_t i = 0; i < MGlobal::MSPOT_LIGHT_MAX_NUMBER; ++i)
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
	DestroyBuffer(pEngine->GetDevice());
}



void MForwardRenderShaderParamSet::UpdateShaderSharedParams(MRenderInfo& info)
{
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
		if (info.pDirectionalLight)
		{
			(*m_pWorldInfoParam->var.GetStruct())[0] = info.pDirectionalLight->GetWorldDirection();
		}

		(*m_pWorldInfoParam->var.GetStruct())[1] = info.pViewport->GetCamera()->GetWorldPosition();

		(*m_pWorldInfoParam->var.GetStruct())[2] = info.pViewport->GetSize();

		(*m_pWorldInfoParam->var.GetStruct())[3] = info.pViewport->GetCamera()->GetZNearFar();

		(*m_pWorldInfoParam->var.GetStruct())[4] = info.fDelta;

		(*m_pWorldInfoParam->var.GetStruct())[5] = info.fGameTime;

		m_pWorldInfoParam->SetDirty();
	}

	if (MShaderConstantParam* pLightParam = m_pLightInfoParam)
	{
		MVariant& varDirLightEnable = (*pLightParam->var.GetStruct())[3];
		if (info.pDirectionalLight)
		{
			varDirLightEnable = 500;
			MVariant& varDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *varDirectionLight.GetStruct();
				{
					cLightStruct[0] = info.pDirectionalLight->GetDiffuseColor().ToVector3();
					cLightStruct[1] = info.pDirectionalLight->GetSpecularColor().ToVector3();
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
			std::vector<MPointLight*> vActivePointLights(MGlobal::MPOINT_LIGHT_MAX_NUMBER);
			info.pScene->FindActivePointLights(info.pViewport->GetCamera()->GetWorldPosition(), vActivePointLights);
			varValidPointLights = 0;

			MVariantArray& vPointLights = *varPointLights.GetArray();
			for (uint32_t i = 0; i < vPointLights.GetMemberCount(); ++i)
			{
				if (MPointLight* pLight = vActivePointLights[i])
				{
					MStruct& cPointLight = *vPointLights[i].GetStruct();
					cPointLight[0] = pLight->GetWorldPosition();
					cPointLight[1] = pLight->GetDiffuseColor().ToVector3();
					cPointLight[2] = pLight->GetSpecularColor().ToVector3();

					cPointLight[3] = pLight->GetConstant();
					cPointLight[4] = pLight->GetLinear();
					cPointLight[5] = pLight->GetQuadratic();

					varValidPointLights = (int)i + 1;
				}
				else break;
			}
		}

		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
		{
			std::vector<MSpotLight*> vActiveSpotLights(MGlobal::MSPOT_LIGHT_MAX_NUMBER);
			info.pScene->FindActiveSpotLights(info.pViewport->GetCamera()->GetWorldPosition(), vActiveSpotLights);
			varValidSpotLights = 0;

			MVariantArray& vSpotLights = *varSpotLights.GetArray();
			for (uint32_t i = 0; i < vSpotLights.GetMemberCount(); ++i)
			{
				if (MSpotLight* pLight = vActiveSpotLights[i])
				{
					Vector3 f3SpotDirection = pLight->GetWorldDirection();
					f3SpotDirection.Normalize();
					MStruct& cSpotLight = *vSpotLights[i].GetStruct();
					cSpotLight[0] = pLight->GetWorldPosition();
					cSpotLight[1] = pLight->GetInnerCutOffRadius();
					cSpotLight[2] = f3SpotDirection;
					cSpotLight[3] = pLight->GetOuterCutOffRadius();
					cSpotLight[4] = pLight->GetDiffuseColor().ToVector3();
					cSpotLight[5] = pLight->GetSpecularColor().ToVector3();

					varValidSpotLights = (int)i + 1;
				}
				else break;
			}
		}

		pLightParam->SetDirty();
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
