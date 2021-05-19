#include "MForwardDebugRenderWork.h"

#include "MScene.h"
#include "MEngine.h"
#include "MSkyBox.h"
#include "MPainter.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MRenderGraph.h"
#include "MTransformCoord.h"

#include "MSkeleton.h"

#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "MRenderGraph.h"
#include "MRenderGraphNode.h"
#include "MRenderGraphTexture.h"

#include "MModelComponent.h"
#include "MSceneComponent.h"
#include "MCameraComponent.h"
#include "MRenderableMeshComponent.h"

M_OBJECT_IMPLEMENT(MForwardDebugRenderWork, MObject)

MForwardDebugRenderWork::MForwardDebugRenderWork()
    : MObject()
	, m_pRenderProgram(nullptr)
	, m_pBoundingDrawMaterial(nullptr)
	, m_pTransformCoordDrawMaterial(nullptr)
	, m_BoundingDrawMesh()
	, m_TransformCoordDrawMesh()
{
}

MForwardDebugRenderWork::~MForwardDebugRenderWork()
{
}

void MForwardDebugRenderWork::Initialize(MIRenderProgram* pRenderProgram)
{
	m_pRenderProgram = pRenderProgram;

	InitializeMaterial();
	InitializeShaderParamSet();
	InitializeRenderGraph();
}

void MForwardDebugRenderWork::Release()
{
	ReleaseRenderGraph();
	ReleaseShaderParamSet();
	ReleaseMaterial();
}


void MForwardDebugRenderWork::InitializeShaderParamSet()
{
}

void MForwardDebugRenderWork::ReleaseShaderParamSet()
{
}

void MForwardDebugRenderWork::InitializeRenderGraph()
{
	MRenderGraph* pRenderGraph = m_pRenderProgram->GetRenderGraph();
	if (!pRenderGraph)
	{
		MLogManager::GetInstance()->Error("MForwardRenderProgram::InitializeRenderGraph error: rg == nullptr");
		return;
	}

	MRenderGraphNode* pForwardNode = pRenderGraph->AddRenderGraphNode("Debug Node");

	MRenderGraphNodeInput* pInputNode = pForwardNode->AppendInput();
	MRenderGraphNodeOutput* pOutputTarget = pForwardNode->AppendOutput();
	pOutputTarget->SetClear(false);

	if (MRenderGraphNodeOutput* pFinalOutput = pRenderGraph->GetFinalOutput())
	{
		pFinalOutput->LinkTo(pInputNode);
		pOutputTarget->SetRenderTexture(pFinalOutput->GetRenderTexture());
		pRenderGraph->SetFinalOutput(pOutputTarget);
	}

	pForwardNode->BindRenderFunction(std::bind(&MForwardDebugRenderWork::Render, this, std::placeholders::_1));
}

void MForwardDebugRenderWork::ReleaseRenderGraph()
{
}

void MForwardDebugRenderWork::InitializeMaterial()
{
	if (!m_pBoundingDrawMaterial)
	{
		MResource* vs = GetEngine()->GetResourceManager()->LoadResource("./Shader/draw3D.mvs");
		MResource* ps = GetEngine()->GetResourceManager()->LoadResource("./Shader/draw3D.mps");
		m_pBoundingDrawMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
		m_pBoundingDrawMaterial->LoadVertexShader(vs);
		m_pBoundingDrawMaterial->LoadPixelShader(ps);
		m_pBoundingDrawMaterial->AddRef();
	}

	if (!m_pTransformCoordDrawMaterial)
	{
		MResource* vs = GetEngine()->GetResourceManager()->LoadResource("./Shader/draw.mvs");
		MResource* ps = GetEngine()->GetResourceManager()->LoadResource("./Shader/draw.mps");
		m_pTransformCoordDrawMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterialResource>();
		m_pTransformCoordDrawMaterial->SetMaterialType(MEMaterialType::EDefault);
		m_pTransformCoordDrawMaterial->SetRasterizerType(MERasterizerType::ECullNone);
		m_pTransformCoordDrawMaterial->LoadVertexShader(vs);
		m_pTransformCoordDrawMaterial->LoadPixelShader(ps);
		m_pTransformCoordDrawMaterial->AddRef();
	}
}

void MForwardDebugRenderWork::ReleaseMaterial()
{
	if (m_pBoundingDrawMaterial)
	{
		m_pBoundingDrawMaterial->SubRef();
		m_pBoundingDrawMaterial = nullptr;
	}

	if (m_pTransformCoordDrawMaterial)
	{
		m_pTransformCoordDrawMaterial->SubRef();
		m_pTransformCoordDrawMaterial = nullptr;
	}

	m_BoundingDrawMesh.DestroyBuffer(GetEngine()->GetDevice());
	m_BoundingDrawMesh.Clean();

	m_TransformCoordDrawMesh.DestroyBuffer(GetEngine()->GetDevice());
	m_TransformCoordDrawMesh.Clean();
}

void MForwardDebugRenderWork::OnDelete()
{
	Release();

	Super::OnDelete();
}

void MForwardDebugRenderWork::Render(MRenderGraphNode* pGraphNode)
{
	MRenderInfo& info = *m_pRenderProgram->GetRenderInfo();

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	DrawPainter(info);

	DrawModelBoundingBox(info);

	info.pRenderer->EndRenderPass(info.pPrimaryCommand);
}

void MForwardDebugRenderWork::DrawModelBoundingBox(MRenderInfo& info)
{
	MComponentGroup* pModelComponentGroup = info.pScene->FindComponents<MModelComponent>();
	if (!pModelComponentGroup)
		return;

	if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pBoundingDrawMaterial))
		return;

	if (!info.pCameraSceneComponent)
		return;

	m_BoundingDrawMesh.DestroyBuffer(GetEngine()->GetDevice());
	m_BoundingDrawMesh.Clean();

	for (MComponent* pComponent : pModelComponentGroup->m_vComponent)
	{
		MModelComponent* pModelComponent = static_cast<MModelComponent*>(pComponent);

		if (pModelComponent->GetBoundingBoxVisiable())
		{
			FillBoundingBoxMesh(info, pModelComponent);
		}
	}

	if (MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord())
	{
		pTransformCoord->FillMesh3D(info.pViewport, m_BoundingDrawMesh);
	}
	

	if (MShaderParamSet* pParam = m_pBoundingDrawMaterial->GetMaterialParamSet())
	{
		if (MShaderConstantParam* pCameraMartixParam = pParam->FindConstantParam("CameraMatrix"))
		{
			if (MStruct* pCBuffer = pCameraMartixParam->var.GetStruct())
			{
				if (Matrix4* pCameraProj = pCBuffer->FindMember<Matrix4>("U_matCamProj"))
				{
					*pCameraProj = info.pViewport->GetCameraInverseProjection();
					pCameraMartixParam->SetDirty();
				}
			}
		}
	}

	info.pRenderer->DrawMesh(info.pPrimaryCommand, &m_BoundingDrawMesh);
}

void MForwardDebugRenderWork::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord();
	if (!pTransformCoord)
		return;

	if (!info.pRenderer->SetUseMaterial(info.pPrimaryCommand, m_pTransformCoordDrawMaterial))
		return;

	m_TransformCoordDrawMesh.DestroyBuffer(GetEngine()->GetDevice());
	m_TransformCoordDrawMesh.Clean();

	pTransformCoord->FillMesh2D(info.pViewport, m_TransformCoordDrawMesh);

	info.pRenderer->DrawMesh(info.pPrimaryCommand, &m_TransformCoordDrawMesh);
}

void MForwardDebugRenderWork::FillBoundingBoxMesh(MRenderInfo& info, MModelComponent* pModelComponent)
{
	MBoundsAABB aabb = pModelComponent->GetBoundsAABB();

	const Vector3& obmin = aabb.m_v3MinPoint;
	const Vector3& obmax = aabb.m_v3MaxPoint;

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
			MPainter3DLine line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(1, 1, 1, 1), 1.0f);
			line.FillData(info.pViewport, m_BoundingDrawMesh);
		}

		MPainter3DLine line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		line.FillData(info.pViewport, m_BoundingDrawMesh);
	}
}

void MForwardDebugRenderWork::DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pMeshIns)
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

void MForwardDebugRenderWork::DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera)
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
