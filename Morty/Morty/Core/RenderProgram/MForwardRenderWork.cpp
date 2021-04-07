#include "MForwardRenderWork.h"

#include "MScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"
#include "MPainter.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MRenderGraph.h"
#include "MTransformCoord.h"

#include "MSkeleton.h"
#include "Model/MModelInstance.h"
#include "Model/MIModelMeshInstance.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

M_OBJECT_IMPLEMENT(MForwardRenderWork, MObject)

MForwardRenderWork::MForwardRenderWork()
    : MObject()
	, m_pRenderProgram(nullptr)
	, m_FrameParamSet()
{
}

MForwardRenderWork::~MForwardRenderWork()
{
}

void MForwardRenderWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeShaderParamSet();
	InitializeRenderGraph();
}

void MForwardRenderWork::Release()
{
	ReleaseRenderGraph();
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

void MForwardRenderWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphTexture* pOutputTargetTexture = pRenderGraph->FindRenderGraphTexture("Output Target");
	if (nullptr == pOutputTargetTexture)
	{
		pOutputTargetTexture = pRenderGraph->AddRenderGraphTexture("Output Target");
		pOutputTargetTexture->SetUsage(METextureUsage::ERenderBack);
		pOutputTargetTexture->SetLayout(METextureLayout::ERGBA8);
		pOutputTargetTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputTargetTexture->SetSize(Vector2(1.0f, 1.0f));
	}

	MRenderGraphTexture* pOutputDepthTexture = pRenderGraph->FindRenderGraphTexture("Output Depth");
	if (nullptr == pOutputDepthTexture)
	{
		pOutputDepthTexture = pRenderGraph->AddRenderGraphTexture("Output Depth");
		pOutputDepthTexture->SetUsage(METextureUsage::ERenderDepth);
		pOutputDepthTexture->SetLayout(METextureLayout::EDepth);
		pOutputDepthTexture->SetSizePolicy(MRenderGraphTexture::ESizePolicy::ERelative);
		pOutputDepthTexture->SetSize(Vector2(1.0f, 1.0f));
	}

	MRenderGraphNode* pForwardNode = pRenderGraph->AddRenderGraphNode("Forward Node");

	MRenderGraphNodeInput* pInputNode = pForwardNode->AppendInput();
	MRenderGraphNodeOutput* pOutputTarget = pForwardNode->AppendOutput();
	MRenderGraphNodeOutput* pOutputDepth = pForwardNode->AppendOutput();

	pOutputTarget->SetClear(true);
	pOutputTarget->SetClearColor(m_pRenderProgram->GetClearColor());
	pOutputTarget->SetRenderTexture(pOutputTargetTexture);
	pRenderGraph->SetFinalOutput(pOutputTarget);

	pOutputDepth->SetClear(true);
	pOutputDepth->SetRenderTexture(pOutputDepthTexture);


	MRenderGraphNode* pShadowMapNode = pRenderGraph->FindRenderGraphNode("Shadow Map Node");

	if (MRenderGraphNodeOutput* pShadowMapOutput = pShadowMapNode->GetOutput(0))
	{
		pInputNode->LinkTo(pShadowMapOutput);

		if (MShaderTextureParam* pShadowMapTextureParam = m_FrameParamSet.m_vTextures[0])
		{
			if (MRenderGraphTexture* pShadowMapTexture = pShadowMapOutput->GetRenderTexture())
			{
				pShadowMapTextureParam->pTexture = pShadowMapTexture->GetRenderTexture();
				pShadowMapTextureParam->SetDirty();
			}
		}
	}

		pForwardNode->BindRenderFunction(std::bind(&MForwardRenderWork::Render, this, std::placeholders::_1));
}

void MForwardRenderWork::ReleaseRenderGraph()
{
}

void MForwardRenderWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardRenderWork::Render(MRenderGraphNode* pGraphNode)
{
	MForwardRenderProgram* pRenderProgram = dynamic_cast<MForwardRenderProgram*>(m_pRenderProgram);
	if (!pRenderProgram)
		return;
	MRenderInfo& info = pRenderProgram->GetRenderInfo();


	m_FrameParamSet.UpdateShaderSharedParams(info);

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	DrawNormalMesh(info);

	DrawPainter(info);

	DrawModelInstance(info);

	//	DrawSkyBox(info);

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MForwardRenderWork::DrawNormalMesh(MRenderInfo& info)
{
	for (MMaterialGroup& group : info.vMaterialRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;

		if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
			continue;

		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_FrameParamSet);

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			DrawMeshInstance(info, pMeshIns);
		}
	}
}

void MForwardRenderWork::DrawMeshInstance(MRenderInfo& info, MIMeshInstance* pMeshInstance)
{
	if (MSkeletonInstance* pSkeletonIns = pMeshInstance->GetSkeletonInstance())
	{
		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pSkeletonIns->GetShaderParamSet());
	}

	info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMeshInstance->GetShaderMeshParamSet());
	info.pRenderer->DrawMesh(info.pPrimaryCommand, pMeshInstance->GetMesh());
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

			if (info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
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

					info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMeshParamSet);
				}

				info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_FrameParamSet);
				info.pRenderer->DrawMesh(info.pPrimaryCommand, pMesh);
			}
		}

	}
}

void MForwardRenderWork::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord();
	pTransformCoord->Render(info.pRenderer, info.pViewport, info.pPrimaryCommand);
}

void MForwardRenderWork::DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(MGlobal::DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
		return;

	info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, &m_FrameParamSet);

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
				info.pRenderer->DrawMesh(info.pPrimaryCommand, &meshs);
			}
		}

		MPainter3DLine line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(info.pPrimaryCommand, &meshs);
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
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(MGlobal::DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, pMaterial))
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
				info.pRenderer->DrawMesh(info.pPrimaryCommand, &meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter3DLine line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(info.pPrimaryCommand, &meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}

}
