#include "MForwardRenderWork.h"

#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"
#include "MPainter.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MTransformCoord.h"

#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

#include "MSkeleton.h"
#include "Model/MModelInstance.h"
#include "Model/MIModelMeshInstance.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_OBJECT_IMPLEMENT(MForwardRenderWork, MObject)

MForwardRenderWork::MForwardRenderWork()
    : MObject()
	, m_pRenderProgram(nullptr)
	, m_FrameParamSet()
	, m_ForwardMeshRenderPass()
{
}

MForwardRenderWork::~MForwardRenderWork()
{
}

void MForwardRenderWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeShaderParamSet();
	InitializeRenderPass();
}

void MForwardRenderWork::Release()
{
	ReleaseRenderPass();
	ReleaseShaderParamSet();
}


void MForwardRenderWork::InitializeShaderParamSet()
{
	m_FrameParamSet.InitializeShaderParamSet(GetEngine());
}

void MForwardRenderWork::ReleaseShaderParamSet()
{
	m_FrameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MForwardRenderWork::InitializeRenderPass()
{
	if (!m_pRenderProgram->GetRenderTarget())
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderPass error: rt == nullptr");
		return;
	}

	//Init RenderPass

	m_ForwardMeshRenderPass.m_vBackDesc.push_back(MPassTargetDescription());
	m_ForwardMeshRenderPass.m_vBackDesc.back().bClearWhenRender = true;
	m_ForwardMeshRenderPass.m_vBackDesc.back().cClearColor = m_pRenderProgram->GetClearColor();

	m_ForwardMeshRenderPass.m_DepthDesc.bClearWhenRender = true;
}

void MForwardRenderWork::ReleaseRenderPass()
{
	GetEngine()->GetDevice()->DestroyRenderPass(&m_ForwardMeshRenderPass);
}

void MForwardRenderWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardRenderWork::SetClearColor(const MColor& cClearColor)
{
	for (MPassTargetDescription& desc : m_ForwardMeshRenderPass.m_vBackDesc)
	{
		desc.cClearColor = cClearColor;
	}
}

void MForwardRenderWork::Render(MRenderInfo& info)
{
	if (MShaderTextureParam* pShadowMapTextureParam = m_FrameParamSet.m_vTextures[0])
	{
		pShadowMapTextureParam->pTexture = info.pShadowMapTexture;
	}

	UpdateShaderSharedParams(info, m_FrameParamSet);

	info.pRenderer->BeginRenderPass(&m_ForwardMeshRenderPass, info.pRenderTarget);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight(), 0.0f, 1.0f);

	DrawNormalMesh(info);

	DrawPainter(info);

	DrawModelInstance(info);

	//	DrawSkyBox(info);

	info.pRenderer->EndRenderPass();

}

void MForwardRenderWork::UpdateShaderSharedParams(MRenderInfo& info, MForwardRenderShaderParamSet& frameParamSet)
{
	if (frameParamSet.m_pWorldMatrixParam)
	{
		MStruct& cStruct = *frameParamSet.m_pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pViewport->GetCameraInverseProjection();
		cStruct[1] = info.m4DirLightInvProj;

		frameParamSet.m_pWorldMatrixParam->SetDirty();
	}

	if (frameParamSet.m_pWorldInfoParam)
	{
		if (info.pDirectionalLight)
		{
			(*frameParamSet.m_pWorldInfoParam->var.GetStruct())[0] = info.pDirectionalLight->GetWorldDirection();
		}

		(*frameParamSet.m_pWorldInfoParam->var.GetStruct())[1] = info.pViewport->GetCamera()->GetWorldPosition();

		(*frameParamSet.m_pWorldInfoParam->var.GetStruct())[2] = info.pViewport->GetSize();

		(*frameParamSet.m_pWorldInfoParam->var.GetStruct())[3] = info.fDelta;

		(*frameParamSet.m_pWorldInfoParam->var.GetStruct())[4] = info.fDelta;

		frameParamSet.m_pWorldInfoParam->SetDirty();
	}

	if (MShaderConstantParam* pLightParam = frameParamSet.m_pLightInfoParam)
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

void MForwardRenderWork::DrawNormalMesh(MRenderInfo& info)
{
	for (MMaterialGroup& group : info.vMaterialRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;

		if (!info.pRenderer->SetUseMaterial(pMaterial))
			continue;

		info.pRenderer->SetShaderParamSet(&m_FrameParamSet);

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			DrawMeshInstance(info.pRenderer, pMeshIns);
		}
	}
}

void MForwardRenderWork::DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance)
{
	if (MSkeletonInstance* pSkeletonIns = pMeshInstance->GetSkeletonInstance())
	{
		pRenderer->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
	}

	pRenderer->SetShaderParamSet(pMeshInstance->GetShaderMeshParamSet());
	pRenderer->DrawMesh(pMeshInstance->GetMesh());
}

void MForwardRenderWork::DrawModelInstance(MRenderInfo& info)
{
	for (MModelInstance* pModelIns : *info.pScene->GetAllModelInstance())
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(info, pModelIns);
		}

		for (MNode* pChild : pModelIns->GetProtectedChildren())
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

void MForwardRenderWork::DrawSkyBox(MRenderInfo& info)
{
	MSkyBox* pSkyBox = info.pScene->GetSkyBox();

	if (pSkyBox)
	{
		if (MIMesh* pMesh = pSkyBox->GetMesh())
		{
			MMaterial* pMaterial = pSkyBox->GetMaterial();

			if (info.pRenderer->SetUseMaterial(pMaterial))
			{
				MShaderParamSet* pMeshParamSet = pSkyBox->GetShaderMeshParamSet();
				MShaderConstantParam* pParam = pSkyBox->GetShaderTransformParam();
				if (pMeshParamSet && pParam)
				{
					MCamera* pCamera = info.pCamera;

					Matrix4 mat(Matrix4::IdentityMatrix);
					Vector3 camPos = info.pViewport->GetCamera()->GetWorldPosition();
					mat.m[0][3] = camPos.x;
					mat.m[1][3] = camPos.y;
					mat.m[2][3] = camPos.z;

					if (MStruct* pSrt = pParam->var.GetStruct())
					{
						*pSrt->FindMember<Matrix4>("U_matWorld") = mat;
						pParam->SetDirty();
					}

					info.pRenderer->SetShaderParamSet(pMeshParamSet);
				}

				info.pRenderer->SetShaderParamSet(&m_FrameParamSet);
				info.pRenderer->DrawMesh(pMesh);
			}
		}

	}
}

void MForwardRenderWork::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord();
	pTransformCoord->Render(info.pRenderer, info.pViewport);
}

void MForwardRenderWork::DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(pMaterial))
		return;

	info.pRenderer->SetShaderParamSet(&m_FrameParamSet);

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

	MMesh<MPainterVertex> meshs;
	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 2; ++i)
		{
			MPainter3DLine line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(1, 1, 1, 1), 1.0f);

			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
			}
		}

		MPainter3DLine line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
		}
	}

	meshs.DestroyBuffer(m_pEngine->GetDevice());
}

void MForwardRenderWork::DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pMeshIns)
{
	// 	MResource* pSphereResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/Sphere.model");
	// 	MMaterialResource* pStaticMeshMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_MODEL_STATIC_MESH);
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

void MForwardRenderWork::DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera)
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
			MPainter3DLine line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(i == 0 ? 0 : 1, 1, 1, 1), 1.0f);

			MMesh<MPainterVertex> meshs;
			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter3DLine line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}

}
