#include "MForwardRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MFunction.h"
#include "MSkeleton.h"
#include "MTaskNode.h"
#include "MTaskNodeOutput.h"

#include "MSceneComponent.h"
#include "MRenderCommand.h"
#include "MRenderableMeshComponent.h"

#include "MObjectSystem.h"
#include "MRenderSystem.h"

#include "MShadowMapRenderWork.h"

MORTY_CLASS_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)

MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_pRenderGraph(nullptr)
	, m_renderInfo()
	, m_frameParamSet()
	, m_pShadowMapWork(nullptr)
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


	UpdateFrameParams(m_renderInfo);

	UpdateRenderGroup(m_renderInfo);
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->UpdateShadowRenderGroup(m_renderInfo);
	}


	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->UpdateShadowParams(m_renderInfo);
	}

	if (m_forwardRenderPass.GetFrameBufferSize() != pViewport->GetSize())
	{
		ResizeForwardRenderPass(pViewport->GetSize(), pRenderSystem->GetDevice());
	}
}

void MForwardRenderProgram::RenderForward(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;

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

MTexture* MForwardRenderProgram::GetOutputTexture()
{
	return m_pFinalOutputTexture;
}

void MForwardRenderProgram::DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{
	auto& materialGroup = info.m_tMaterialGroupMesh;
	for (auto& pr : materialGroup)
	{
		MMaterial* pMaterial = pr.first;
		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		pCommand->SetUseMaterial(pMaterial);
		pCommand->SetShaderParamSet(info.pFrameShaderParamSet);

		for (MRenderableMeshComponent* pMeshComponent : vMesh)
		{
			if (MSkeletonInstance* pSkeletonIns = pMeshComponent->GetSkeletonInstance())
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

	MTaskNode* pRenderReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pRenderReadyTask->SetThreadType(METhreadType::EAny);
	pRenderReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderReady, this));

 	MTaskNode* pRenderShadowTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Shadowmap");
 	pRenderShadowTask->SetThreadType(METhreadType::EAny);
 	pRenderShadowTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderShadow, this));

	MTaskNode* pRenderForwardTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Forward");
	pRenderForwardTask->SetThreadType(METhreadType::EAny);
	pRenderForwardTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderForward, this));

	/*
		RenderReady --> RenderShadowmap --> RenderForward --> RenderTransparent --> output				
	*/

 	pRenderReadyTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
 	pRenderShadowTask->AppendOutput()->LinkTo(pRenderForwardTask->AppendInput());

	m_frameParamSet.InitializeShaderParamSet(GetEngine());

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

	m_frameParamSet.ReleaseShaderParamSet(GetEngine());

	ReleaseRenderPass();
}

void MForwardRenderProgram::InitializeRenderPass()
{
	Vector2 v2Size = Vector2(512.0f, 512.0f);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();


	MTexture* pRenderTarget = MTexture::CreateRenderTarget();
	pRenderTarget->SetSize(v2Size);
	pRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	m_forwardRenderPass.m_vBackTextures.push_back(pRenderTarget);
	m_forwardRenderPass.m_vBackDesc.push_back(MPassTargetDescription(true, MColor(0.0f, 0.0f, 0.0f, 1.0)));


	m_forwardRenderPass.m_pDepthTexture = MTexture::CreateShadowMap();
	m_forwardRenderPass.m_pDepthTexture->SetSize(v2Size);
	m_forwardRenderPass.m_pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_forwardRenderPass.m_DepthDesc.bClearWhenRender = true;
	m_forwardRenderPass.m_DepthDesc.cClearColor = MColor::White;

	m_forwardRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

	m_pFinalOutputTexture = pRenderTarget;
}

void MForwardRenderProgram::ReleaseRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	for (MTexture* pTexture : m_forwardRenderPass.m_vBackTextures)
	{
		pTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete pTexture;
		pTexture = nullptr;
	}
	if (m_forwardRenderPass.m_pDepthTexture)
	{
		m_forwardRenderPass.m_pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		delete m_forwardRenderPass.m_pDepthTexture;
		m_forwardRenderPass.m_pDepthTexture = nullptr;
	}

	m_forwardRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MForwardRenderProgram::UpdateRenderGroup(MRenderInfo& info)
{
	MScene* pScene = info.pViewport->GetScene();

	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	MComponentGroup<MRenderableMeshComponent>* pMeshComponents = pScene->FindComponents<MRenderableMeshComponent>();

	if (!pMeshComponents)
		return;

	for (MRenderableMeshComponent& meshComp : pMeshComponents->m_vComponent)
	{
		MMaterial* pMaterial = meshComp.GetMaterial();
		auto& meshes = m_renderInfo.m_tMaterialGroupMesh[pMaterial];

		MSceneComponent* pSceneComponent = meshComp.GetEntity()->GetComponent<MSceneComponent>();

		if (!pSceneComponent->GetVisibleRecursively())
			continue;

		const MBoundsAABB* pBounds = meshComp.GetBoundsAABB();

		if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum().ContainTest(*pBounds))
			continue;

		meshes.push_back(&meshComp);

		pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
	}

	m_renderInfo.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}

void MForwardRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_frameParamSet.UpdateShaderSharedParams(info);
	info.pFrameShaderParamSet = &m_frameParamSet;
}

void MForwardRenderProgram::ResizeForwardRenderPass(const Vector2& v2Size, MIDevice* pDevice)
{
	for (MTexture* pTexture : m_forwardRenderPass.m_vBackTextures)
	{
		pTexture->SetSize(v2Size);
		pTexture->DestroyBuffer(pDevice);
		pTexture->GenerateBuffer(pDevice);
	}

	if (m_forwardRenderPass.m_pDepthTexture)
	{
		m_forwardRenderPass.m_pDepthTexture->SetSize(v2Size);
		m_forwardRenderPass.m_pDepthTexture->DestroyBuffer(pDevice);
		m_forwardRenderPass.m_pDepthTexture->GenerateBuffer(pDevice);
	}

	m_forwardRenderPass.Resize(pDevice);
}

void MForwardRenderProgram::SubmitCommand(MTaskNode* pTaskNode)
{
	MIRenderCommand* pPrimaryCommand = m_renderInfo.pPrimaryRenderCommand;

	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();
}
