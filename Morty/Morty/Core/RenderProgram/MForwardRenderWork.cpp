#include "MForwardRenderWork.h"

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
	MRenderInfo& info = *m_pRenderProgram->GetRenderInfo();


	m_FrameParamSet.UpdateShaderSharedParams(info);

	info.pRenderer->BeginRenderPass(info.pPrimaryCommand, pGraphNode->GetRenderPass(), info.unFrameIndex);

	Vector2 v2LeftTop = info.pViewport->GetLeftTop();
	info.pRenderer->SetViewport(info.pPrimaryCommand, MViewportInfo(v2LeftTop.x, v2LeftTop.y, info.pViewport->GetWidth(), info.pViewport->GetHeight()));
	info.pRenderer->SetScissor(info.pPrimaryCommand, MScissorInfo(0.0f, 0.0f, info.pViewport->GetWidth(), info.pViewport->GetHeight()));

	DrawNormalMesh(info);

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

		for (MRenderableMeshComponent* pMeshComponent : group.m_vMeshComponents)
		{
			DrawMeshComponent(info, pMeshComponent);
		}
	}
}

void MForwardRenderWork::DrawMeshComponent(MRenderInfo& info, MRenderableMeshComponent* pMeshComponent)
{
	if (MSkeletonInstance* pSkeletonIns = pMeshComponent->GetSkeletonInstance())
	{
		info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pSkeletonIns->GetShaderParamSet());
	}

	info.pRenderer->SetShaderParamSet(info.pPrimaryCommand, pMeshComponent->GetShaderMeshParamSet());
	info.pRenderer->DrawMesh(info.pPrimaryCommand, pMeshComponent->GetMesh());
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
					MSceneComponent* pCameraSceneComponent = info.pCameraSceneComponent;

					Matrix4 mat(Matrix4::IdentityMatrix);
					Vector3 camPos = pCameraSceneComponent->GetWorldPosition();
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
