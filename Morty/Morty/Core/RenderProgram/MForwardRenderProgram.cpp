#include "MForwardRenderProgram.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MIRenderer.h"

#include "MScene.h"
#include "MViewport.h"
#include "MTexture.h"

#include "MPainter.h"
#include "MTexture.h"
#include "MRenderStructure.h"
#include "MTransformCoord.h"

#include "MCamera.h"
#include "MSkyBox.h"
#include "MSkeleton.h"
#include "Model/MModelInstance.h"
#include "Model/MIMeshInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

#include "MSkeleton.h"
#include "Material/MMaterialResource.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"

#include "Shader/MShaderBuffer.h"

#include "MForwardShadowMapWork.h"
#include "MForwardTransparentWork.h"

#include <algorithm>


M_OBJECT_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)


MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_FrameParamSet(1)
	
	, m_pWorldMatrixParam(nullptr)
	, m_pWorldInfoParam(nullptr)
	, m_pLightInfoParam(nullptr)
	
	, m_pDefaultSampleParam(nullptr)
	, m_pLessEqualSampleParam(nullptr)
	, m_pGreaterEqualSampleParam(nullptr)

	, m_pShadowTextureParam(nullptr)
	, m_pTransparentFrontTextureParam(nullptr)
	, m_pTransparentBackTextureParam(nullptr)

	, m_pShadowMapWork(nullptr)
	, m_pTransparentWork(nullptr)
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::InitializeShaderParamSet()
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
	for (uint32_t i = 0; i < MPOINT_LIGHT_MAX_NUMBER; ++i)
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
	for (uint32_t i = 0; i < MSPOT_LIGHT_MAX_NUMBER; ++i)
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
	m_pLessEqualSampleParam = new MShaderSampleParam();
	m_pLessEqualSampleParam->unSet = 1;
	m_pLessEqualSampleParam->unBinding = 4;
	m_pGreaterEqualSampleParam = new MShaderSampleParam();
	m_pGreaterEqualSampleParam->unSet = 1;
	m_pGreaterEqualSampleParam->unBinding = 5;

	m_pShadowTextureParam = new MShaderTextureParam();
	m_pShadowTextureParam->unSet = 1;
	m_pShadowTextureParam->unBinding = 6;
	m_pTransparentFrontTextureParam = new MShaderTextureParam();
	m_pTransparentFrontTextureParam->unSet = 1;
	m_pTransparentFrontTextureParam->unBinding = 7;
	m_pTransparentBackTextureParam = new MShaderTextureParam();
	m_pTransparentBackTextureParam->unSet = 1;
	m_pTransparentBackTextureParam->unBinding = 8;

	m_FrameParamSet.m_vParams.push_back(m_pWorldMatrixParam);
	m_FrameParamSet.m_vParams.push_back(m_pWorldInfoParam);
	m_FrameParamSet.m_vParams.push_back(m_pLightInfoParam);


	m_FrameParamSet.m_vSamples.push_back(m_pDefaultSampleParam);
	m_FrameParamSet.m_vSamples.push_back(m_pLessEqualSampleParam);
	m_FrameParamSet.m_vSamples.push_back(m_pGreaterEqualSampleParam);


	m_FrameParamSet.m_vTextures.push_back(m_pShadowTextureParam);
	m_FrameParamSet.m_vTextures.push_back(m_pTransparentFrontTextureParam);
	m_FrameParamSet.m_vTextures.push_back(m_pTransparentBackTextureParam);
}

void MForwardRenderProgram::ReleaseShaderParamSet()
{
	m_FrameParamSet.ClearAndDestroy(GetEngine()->GetDevice());
}

void MForwardRenderProgram::Render(MIRenderer* pRenderer, MIRenderTarget* pRenderTarget, const std::vector<MViewport*>& vViewports)
{
	MRenderInfo info;
	memset(&info, 0, sizeof(MRenderInfo));

	info.unFrameIndex = pRenderer->GetFrameIndex();
	info.pRenderTarget = pRenderTarget;
	info.pRenderer = pRenderer;

	pRenderer->RenderBegin(pRenderTarget);
 	
	for (MViewport* pViewport : vViewports)
	{
		RenderWithViewport(info, pViewport);
	}

	pRenderer->RenderEnd(pRenderTarget);
}

void MForwardRenderProgram::RenderWithViewport(MRenderInfo info, MViewport* pViewport)
{
 	info.pViewport = pViewport;
 	info.pCamera = pViewport->GetCamera();
 	info.pScene = pViewport->GetScene();

	GenerateRenderGroup(info);

	if (m_pShadowMapWork)
	{
		if (MShaderTextureParam* pShadowMapTextureParam = m_FrameParamSet.m_vTextures[0])
		{
			m_pShadowMapWork->DrawShadowMap(info);
			pShadowMapTextureParam->pTexture = m_pShadowMapWork->GetShadowMap(info.unFrameIndex);
		}
	}


	info.pRenderer->BeginRenderPass(info.pRenderTarget);


	Vector2 v2LeftTop = pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);

	UpdateShaderSharedParams(info);
	DrawNormalMesh(info);


	info.pRenderer->EndRenderPass(info.pRenderTarget);






	if (m_pTransparentWork)
		m_pTransparentWork->DrawTransparentMesh(info);

	//DrawSkyBox(info);
//	DrawModelInstance(info);
//	DrawPainter(info);
}

void MForwardRenderProgram::OnCreated()
{
	Super::OnCreated();

	InitializeShaderParamSet();

	m_pShadowMapWork = GetEngine()->GetObjectManager()->CreateObject<MForwardShadowMapWork>();
	m_pShadowMapWork->SetProgram(this);
// 
// 	m_pTransparentWork = GetEngine()->GetObjectManager()->CreateObject<MForwardTransparentWork>();
// 	m_pTransparentWork->SetProgram(this);
}

void MForwardRenderProgram::OnDelete()
{
	ReleaseShaderParamSet();

	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->DeleteLater();
		m_pShadowMapWork = nullptr;
	}

	if (m_pTransparentWork)
	{
		m_pTransparentWork->DeleteLater();
		m_pTransparentWork = nullptr;
	}

	Super::OnDelete();
}

void MForwardRenderProgram::UpdateShaderSharedParams(MRenderInfo& info)
{
	if (m_pWorldMatrixParam)
	{
		MStruct& cStruct = *m_pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pViewport->GetCameraInverseProjection();
		cStruct[1] = info.m4DirLightInvProj;

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
			std::vector<MPointLight*> vActivePointLights(MPOINT_LIGHT_MAX_NUMBER);
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


					cPointLight[3] = 1.0f;
					cPointLight[4] = 0.022f;
					cPointLight[5] = 0.0019f;

					varValidPointLights = (int)i + 1;
				}
				else break;
			}
		}

		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
		{
			std::vector<MSpotLight*> vActiveSpotLights(MSPOT_LIGHT_MAX_NUMBER);
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

void MForwardRenderProgram::DrawNormalMesh(MRenderInfo& info)
{
	for (MMaterialGroup& group : info.vMaterialRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;
		//Ê¹ÓÃ²ÄÖÊ
		if (!info.pRenderer->SetUseMaterial(pMaterial))
			continue;

		info.pRenderer->SetShaderParamSet(&m_FrameParamSet);
		info.pRenderer->SetShaderParamSet(pMaterial->GetMaterialParamSet());

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			DrawMeshInstance(info.pRenderer, pMeshIns);
		}
	}
}

void MForwardRenderProgram::DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance)
{
// 	if (MSkeletonInstance* pSkeletonIns = pMeshInstance->GetSkeletonInstance())
// 	{
// 		MStruct& cAnimationStruct = *pAnimationParam->var.GetStruct();
// 		MVariantArray& cBonesArray = *cAnimationStruct[0].GetArray();
// 
// 		const std::vector<MBone>& bones = pSkeletonIns->GetAllBones();
// 		uint32_t size = bones.size();
// 		if (size > MBONES_MAX_NUMBER) size = MBONES_MAX_NUMBER;
// 
// 		for (uint32_t i = 0; i < size; ++i)
// 		{
// 			cBonesArray[i] = bones[i].m_matWorldTransform;
// 		}
// 
// 		pAnimationParam->SetDirty();
// 		pRenderer->SetShaderParam(*pAnimationParam);
// 	}


	pRenderer->SetShaderParamSet(pMeshInstance->GetShaderMeshParamSet());
	pRenderer->DrawMesh(pMeshInstance->GetMesh());
}

void MForwardRenderProgram::DrawModelInstance(MRenderInfo& info)
{
	for (MModelInstance* pModelIns : *info.pScene->GetAllModelInstance())
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(info, pModelIns);
		}

		for (MNode* pChild : pModelIns->GetFixedChildren())
		{
			if (MIModelMeshInstance* pMeshIns = dynamic_cast<MIModelMeshInstance*>(pChild))
			{
				if (pMeshIns->GetDrawBoundingSphere())
				{
					DrawBoundingSphere(info, pMeshIns);
				}
			}
		}
	}


}

void MForwardRenderProgram::DrawSkyBox(MRenderInfo& info)
{
// 	MSkyBox* pSkyBox = info.pScene->GetSkyBox();
// 
// 	if (pSkyBox)
// 	{
// 		if (MIMesh* pMesh = pSkyBox->GetMesh())
// 		{
// 			MMaterial* pMaterial = pSkyBox->GetMaterial();
// 
// 			if (MShaderConstantParam* pMeshParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
// 			{
// 				MStruct& cStruct = *pMeshParam->var.GetStruct();
// 				Matrix4 mat(Matrix4::IdentityMatrix);
// 				Vector3 camPos = info.pViewport->GetCamera()->GetWorldPosition();
// 				mat.m[0][3] = camPos.x;
// 				mat.m[1][3] = camPos.y;
// 				mat.m[2][3] = camPos.z;
// 				cStruct[0] = mat;
// 
// 				pMeshParam->SetDirty();
// 				info.pRenderer->SetShaderParam(*pMeshParam);
// 			}
// 
// 			if (info.pRenderer->SetUseMaterial(pMaterial, true))
// 			{
// 				info.pRenderer->DrawMesh(pMesh);
// 			}
// 		}
// 
// 	}
}

void MForwardRenderProgram::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord();
	pTransformCoord->Render(info.pRenderer, info.pViewport);
}

void MForwardRenderProgram::DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(pMaterial))
		return;

	const MBoundsAABB* pAABB = pModelIns->GetBoundsAABB();

	const Vector3& obmin = pAABB->m_v3MinPoint;
	const Vector3& obmax = pAABB->m_v3MaxPoint;

	Vector3 list[] = {
		Vector3(obmin.x, obmin.y, obmin.z),
		Vector3(obmax.x, obmin.y, obmin.z),
		Vector3(obmax.x, obmax.y, obmin.z),
		Vector3(obmin.x, obmax.y, obmin.z),

		Vector3(obmin.x, obmin.y, obmax.z),
		Vector3(obmax.x, obmin.y, obmax.z),
		Vector3(obmax.x, obmax.y, obmax.z),
		Vector3(obmin.x, obmax.y, obmax.z),
	};

	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 2; ++i)
		{
			MPainter2DLine3D line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(1, 1, 1, 1), 1.0f);

			MMesh<MPainterVertex> meshs;
			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter2DLine3D line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}
}

void MForwardRenderProgram::DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pMeshIns)
{
// 	MResource* pSphereResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/Sphere.model");
// 	MMaterialResource* pStaticMeshMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);
// 
// 	MMaterial& mat = *pStaticMeshMaterialRes;
// 	mat.SetRasterizerType(MERasterizerType::ECullNone);
// 
// 	MShaderConstantParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
// 	if (nullptr == pMeshMatrixParam)
// 		return;
// 
// 	if (!info.pRenderer->SetUseMaterial(pStaticMeshMaterialRes))
// 		return;
// 
// 	MTransform trans;
// 	if (MBoundsSphere* pSphere = pMeshIns->GetBoundsSphere())
// 	{
// 		float fScale = pSphere->m_fRadius / 3.8f;
// 		trans.SetPosition(pSphere->m_v3CenterPoint);
// 		trans.SetScale(Vector3(fScale, fScale, fScale));
// 	}
// 
// 	Matrix4 worldTrans = trans.GetMatrix();
// 	MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
// 	cStruct[0] = worldTrans;
// 
// 	pMeshMatrixParam->SetDirty();
// 	info.pRenderer->SetShaderParam(*pMeshMatrixParam);
// 
// 	if (MModelResource* pModelResource = dynamic_cast<MModelResource*>(pSphereResource))
// 	{
// 		for (MMeshResource* pMeshRes : pModelResource->GetMeshResources())
// 		{
// 			info.pRenderer->DrawMesh(pMeshRes->GetMesh());
// 		}
// 	}
}

void MForwardRenderProgram::DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(pMaterial))
		return;


	Vector3 list[8];
	info.pViewport->GetCameraFrustum(list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7]);

	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 2; ++i)
		{
			MPainter2DLine3D line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(i == 0 ? 0 : 1, 1, 1, 1), 1.0f);

			MMesh<MPainterVertex> meshs;
			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter2DLine3D line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}

}

void MForwardRenderProgram::GenerateRenderGroup(MRenderInfo& info)
{
	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::vector<MMaterialGroup*>& matGroups = info.pScene->GetMaterialGroup();
	for (MMaterialGroup* pMaterialGroup : matGroups)
	{
		MMaterialGroup* pRenderGroup = nullptr;

		if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::EDefault)
		{
			info.vMaterialRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vMaterialRenderGroup.back();
		}
		else if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::ETransparent)
		{
			info.vTransparentRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vTransparentRenderGroup.back();
		}


		pRenderGroup->m_pMaterial = pMaterialGroup->m_pMaterial;

		for (MIMeshInstance* pMeshIns : pMaterialGroup->m_vMeshInstances)
		{
			if (!pMeshIns->GetVisibleRecursively())
				continue;

			if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum()->ContainTest(*pMeshIns->GetBoundsAABB()))
				continue;

			pRenderGroup->m_vMeshInstances.push_back(pMeshIns);

			const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	info.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
