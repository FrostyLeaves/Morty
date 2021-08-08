#include "MForwardRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MFunction.h"
#include "MSkeleton.h"
#include "MRenderTaskNode.h"
#include "MRenderTaskNodeOutput.h"

#include "MSceneComponent.h"
#include "MRenderCommand.h"
#include "MRenderableMeshComponent.h"

#include "MObjectSystem.h"
#include "MRenderSystem.h"

MORTY_CLASS_IMPLEMENT(MForwardRenderProgram, MIRenderProgram)

MForwardRenderProgram::MForwardRenderProgram()
	: MIRenderProgram()
	, m_pRenderGraph(nullptr)
	, m_renderInfo()
	, m_frameParamSet()
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::Render()
{
	if (!GetViewport())
		return;

	m_pRenderGraph->Run();
}

void MForwardRenderProgram::RenderReady(MTaskNode* pTaskNode)
{
	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();
	MViewport* pViewport = GetViewport();

	m_renderInfo = MRenderInfo();
	m_renderInfo.pViewport = pViewport;

	GenerateRenderGroup(m_renderInfo);

	UpdateFrameParams(m_renderInfo);


	if (m_pOutputTexture->GetSize() != pViewport->GetSize())
	{
		Vector2 v2Size = pViewport->GetSize();
		m_pOutputTexture->SetSize(v2Size);
		m_pOutputTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pOutputTexture->GenerateBuffer(pRenderSystem->GetDevice());

		m_pDepthTexture->SetSize(v2Size);
		m_pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());

		for (MTaskNode* pTaskNode : m_pRenderGraph->GetAllNodes())
		{
			if (MRenderTaskNode* pRenderTaskNode = pTaskNode->DynamicCast<MRenderTaskNode>())
			{
				if (MRenderPass* pRenderpass = pRenderTaskNode->GetRenderPass())
				{
					pRenderpass->Resize(pRenderSystem->GetDevice());
				}
			}
		}
	}
}

void MForwardRenderProgram::RenderShadow(MTaskNode* pTaskNode)
{

}

void MForwardRenderProgram::RenderForward(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MRenderTaskNode* pRenderTaskNode = pTaskNode->DynamicCast<MRenderTaskNode>();

	MRenderPass* pRenderpass = pRenderTaskNode->GetRenderPass();

	MIRenderCommand* pCommand = pRenderDevice->CreateRenderCommand();

	MViewport* pViewport = m_renderInfo.pViewport;

	pCommand->RenderCommandBegin();

	pCommand->BeginRenderPass(pRenderpass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	DrawStaticMesh(m_renderInfo, pCommand);

	pCommand->EndRenderPass();

	pCommand->RenderCommandEnd();


	pRenderDevice->SubmitCommand(pCommand);

}

void MForwardRenderProgram::DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand)
{
	auto& materialGroup = m_renderInfo.m_tMaterialGroupMesh;
	for (auto& pr : materialGroup)
	{
		MMaterial* pMaterial = pr.first;
		std::vector<MRenderableMeshComponent*>& vMesh = pr.second;

		pCommand->SetUseMaterial(pMaterial);
		pCommand->SetShaderParamSet(&m_frameParamSet);

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

	MTaskNode* pReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pReadyTask->SetThreadType(METhreadType::EAny);
	pReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderReady, this));

	MTaskNode* pShadowmapTask = m_pRenderGraph->AddNode<MRenderTaskNode>("Render_Shadowmap");
	pShadowmapTask->SetThreadType(METhreadType::EAny);
	pShadowmapTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderShadow, this));

	MRenderTaskNode* pForwardTask = m_pRenderGraph->AddNode<MRenderTaskNode>("Render_Forward");
	pForwardTask->SetThreadType(METhreadType::EAny);
	pForwardTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::RenderForward, this));

	pReadyTask->AppendOutput()->LinkTo(pShadowmapTask->AppendInput());

	if (MRenderTaskNodeOutput* pShadowmapOutput = pShadowmapTask->AppendOutput<MRenderTaskNodeOutput>())
	{
		m_pShadowMapTexture = MTexture::CreateShadowMap();
		m_pShadowMapTexture->SetSize(Vector2(1024.0, 1024.0));
		m_pShadowMapTexture->GenerateBuffer(pRenderSystem->GetDevice());
		pShadowmapOutput->SetTexture(m_pShadowMapTexture);

		pShadowmapOutput->LinkTo(pForwardTask->AppendInput());
	}

	if (MRenderTaskNodeOutput* pForwardBackOutput = pForwardTask->AppendOutput())
	{
		m_pOutputTexture = new MTexture();
		m_pOutputTexture->SetMipmapsEnable(false);
		m_pOutputTexture->SetReadable(false);
		m_pOutputTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
		m_pOutputTexture->SetShaderUsage(METextureShaderUsage::ESampler);
		m_pOutputTexture->SetSize(Vector2(512, 512));
		m_pOutputTexture->SetTextureLayout(METextureLayout::ERGBA8);
		m_pOutputTexture->GenerateBuffer(pRenderSystem->GetDevice());

		pForwardBackOutput->SetTexture(m_pOutputTexture);
		pForwardBackOutput->SetClear(true);
		pForwardBackOutput->SetClearColor(MColor(0.2, 0.2, 0.2, 1.0));
	}

	if (MRenderTaskNodeOutput* pForwardDepthOutput = pForwardTask->AppendOutput())
	{
		m_pDepthTexture = new MTexture();
		m_pDepthTexture->SetMipmapsEnable(false);
		m_pDepthTexture->SetReadable(false);
		m_pDepthTexture->SetRenderUsage(METextureRenderUsage::ERenderDepth);
		m_pDepthTexture->SetShaderUsage(METextureShaderUsage::ESampler);
		m_pDepthTexture->SetSize(Vector2(512, 512));
		m_pDepthTexture->SetTextureLayout(METextureLayout::EDepth);
		m_pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());

		pForwardDepthOutput->SetTexture(m_pDepthTexture);
		pForwardDepthOutput->SetClear(true);
		pForwardDepthOutput->SetClearColor(MColor(0.0, 0.0, 0.0, 1.0));
	}


	m_frameParamSet.InitializeShaderParamSet(GetEngine());
}

void MForwardRenderProgram::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	m_pRenderGraph->DeleteLater();
	m_pRenderGraph = nullptr;

	m_pShadowMapTexture->DestroyBuffer(pRenderSystem->GetDevice());
	delete m_pShadowMapTexture;
	m_pShadowMapTexture = nullptr;

	m_pOutputTexture->DestroyBuffer(pRenderSystem->GetDevice());
	delete m_pOutputTexture;
	m_pOutputTexture = nullptr;

	m_pDepthTexture->DestroyBuffer(pRenderSystem->GetDevice());
	delete m_pDepthTexture;
	m_pDepthTexture = nullptr;


	m_frameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MForwardRenderProgram::GenerateRenderGroup(MRenderInfo& info)
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
}
