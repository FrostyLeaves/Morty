#include "MForwardRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MFunction.h"
#include "MSkeleton.h"
#include "MTaskNode.h"
#include "MMaterial.h"
#include "MTaskNodeOutput.h"

#include "MSceneComponent.h"
#include "MRenderCommand.h"
#include "MRenderableMeshComponent.h"

#include "MObjectSystem.h"
#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MShadowMapRenderWork.h"
#include "MTransparentRenderWork.h"

#include "MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)

MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_pRenderGraph(nullptr)
	, m_renderInfo()
	, m_frameParamSet()
	, m_pShadowMapWork(nullptr)
	, m_pTransparentWork(nullptr)
	, m_nFrameIndex(0)
	, m_pFinalOutputTexture(nullptr)
	, m_forwardRenderPass()
	, m_pPrimaryCommand(nullptr)
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	m_pPrimaryCommand = pPrimaryCommand;
	m_pRenderGraph->Run();
}

void MForwardRenderProgram::RenderReady(MTaskNode* pTaskNode)
{
	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();
	MViewport* pViewport = GetViewport();

	m_renderInfo = MRenderInfo();
	m_renderInfo.nFrameIndex = m_nFrameIndex++;
	m_renderInfo.pViewport = pViewport;
	m_renderInfo.pPrimaryRenderCommand = m_pPrimaryCommand;


	//Update RenderInfo
	m_renderInfo.CollectRenderMesh();

	if (m_pShadowMapWork)
	{
		m_renderInfo.CollectShadowMesh();
	}


	//Update Shader Params.
	UpdateFrameParams(m_renderInfo);
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->UpdateShadowParams(m_renderInfo);
	}

	//Resize FrameBuffer.
	if (m_forwardRenderPass.GetFrameBufferSize() != pViewport->GetSize())
	{
		pRenderSystem->ResizeFrameBuffer(m_forwardRenderPass, pViewport->GetSize());

		if (m_pTransparentWork)
		{
			m_pTransparentWork->Resize(pViewport->GetSize());
			m_pTransparentWork->SetRenderTarget(GetOutputTexture(), m_forwardRenderPass.GetDepthTexture());
		}
	}
}

void MForwardRenderProgram::RenderForward(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;


	if (m_pShadowMapWork)
	{
		if (MTexture* pShadowMap = m_pShadowMapWork->GetShadowMap())
		{
			pCommand->SetRenderToTextureBarrier({ pShadowMap });
		}
	}

	pCommand->BeginRenderPass(&m_forwardRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	DrawStaticMesh(m_renderInfo, pCommand);

	pCommand->EndRenderPass();
}

void MForwardRenderProgram::RenderShadow(MTaskNode* pTaskNode)
{
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->RenderShadow(m_renderInfo);

		MTexture* pShadowMap = m_pShadowMapWork->GetShadowMap();

		m_frameParamSet.SetShadowMapTexture(pShadowMap);
	}
}

void MForwardRenderProgram::RenderTransparent(MTaskNode* pTaskNode)
{
	if (m_pTransparentWork)
	{
		m_pTransparentWork->RenderDepthPeel(m_renderInfo);
		m_pTransparentWork->Render(m_renderInfo);
	}
}

MTexture* MForwardRenderProgram::GetOutputTexture()
{
	return m_pFinalOutputTexture;
}

std::vector<MTexture*> MForwardRenderProgram::GetOutputTextures()
{
	std::vector<MTexture*> vResult;

	if (m_pFinalOutputTexture)
		vResult.push_back(m_pFinalOutputTexture);

	if (m_pShadowMapWork)
	{
		if (MTexture* pTexture = m_pShadowMapWork->GetShadowMap())
		{
			vResult.push_back(pTexture);
		}
	}

	return vResult;
}

void MForwardRenderProgram::DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{
	auto& materialGroup = info.m_tMaterialGroupMesh;
	for (auto& pr : materialGroup)
	{
		std::shared_ptr<MMaterial> pMaterial = pr.first;
		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		pCommand->SetUseMaterial(pMaterial);
		pCommand->SetShaderParamSet(info.pFrameShaderParamSet);

		for (MRenderableMeshComponent* pMeshComponent : vMesh)
		{
			if (std::shared_ptr<MSkeletonInstance> pSkeletonIns = pMeshComponent->GetSkeletonInstance())
			{
				pCommand->SetShaderParamSet(pSkeletonIns->GetShaderParamSet());
			}
			pCommand->SetShaderParamSet(pMeshComponent->GetShaderMeshParamSet());
			pCommand->DrawMesh(pMeshComponent->GetMesh());
		}
	}
}

void MForwardRenderProgram::OnCreated()
{
	Super::OnCreated();

	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pRenderGraph = pObjectSystem->CreateObject<MTaskGraph>();
	m_pShadowMapWork = pObjectSystem->CreateObject<MShadowMapRenderWork>();
	m_pTransparentWork = pObjectSystem->CreateObject<MTransparentRenderWork>();

	MTaskNode* pRenderReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pRenderReadyTask->SetThreadType(METhreadType::EAny);
	pRenderReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderReady, this));

 	MTaskNode* pRenderShadowTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Shadowmap");
 	pRenderShadowTask->SetThreadType(METhreadType::EAny);
 	pRenderShadowTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderShadow, this));

	MTaskNode* pRenderForwardTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Forward");
	pRenderForwardTask->SetThreadType(METhreadType::EAny);
	pRenderForwardTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderForward, this));

	MTaskNode* pRenderTransparentTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Transparent");
	pRenderTransparentTask->SetThreadType(METhreadType::EAny);
	pRenderTransparentTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderTransparent, this));

	/*
		RenderReady --> RenderShadowmap --> RenderForward --> RenderTransparent --> output				
	*/

 	pRenderReadyTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
 	pRenderShadowTask->AppendOutput()->LinkTo(pRenderForwardTask->AppendInput());
	pRenderForwardTask->AppendOutput()->LinkTo(pRenderTransparentTask->AppendInput());

	InitializeFrameShaderParams();
	InitializeRenderPass();
}

void MForwardRenderProgram::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	m_pRenderGraph->DeleteLater();
	m_pRenderGraph = nullptr;

	m_pShadowMapWork->DeleteLater();
	m_pShadowMapWork = nullptr;

	ReleaseFrameShaderParams();
	ReleaseRenderPass();
}

void MForwardRenderProgram::InitializeRenderPass()
{
	Vector2 v2Size = Vector2(512.0f, 512.0f);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();


	MTexture* pRenderTarget = MTexture::CreateRenderTarget();
	pRenderTarget->SetSize(v2Size);
	pRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	m_forwardRenderPass.AddBackTexture(pRenderTarget, { true, MColor(0.0f, 0.0f, 0.0f, 1.0) });


	MTexture* pShadowMapTexture = MTexture::CreateShadowMap();
	pShadowMapTexture->SetSize(v2Size);
	pShadowMapTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_forwardRenderPass.SetDepthTexture(pShadowMapTexture, { true, MColor::White });

	m_forwardRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

	m_pFinalOutputTexture = pRenderTarget;
}

void MForwardRenderProgram::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	for (MBackTexture& tex : m_forwardRenderPass.m_vBackTextures)
	{
		tex.pTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete tex.pTexture;
		tex.pTexture = nullptr;
	}
	if (MTexture* pDepthTexture = m_forwardRenderPass.GetDepthTexture())
	{
		pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete pDepthTexture;
		pDepthTexture = nullptr;
		m_forwardRenderPass.SetDepthTexture(nullptr, {});
	}

	m_forwardRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MForwardRenderProgram::InitializeFrameShaderParams()
{
	m_frameParamSet.InitializeShaderParamSet(GetEngine());
}

void MForwardRenderProgram::ReleaseFrameShaderParams()
{
	m_frameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MForwardRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_frameParamSet.UpdateShaderSharedParams(info);
	info.pFrameShaderParamSet = &m_frameParamSet;
}
