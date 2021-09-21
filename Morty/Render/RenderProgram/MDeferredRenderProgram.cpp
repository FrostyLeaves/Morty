#include "MDeferredRenderProgram.h"

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

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

MDeferredRenderProgram::MDeferredRenderProgram()
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

MDeferredRenderProgram::~MDeferredRenderProgram()
{
}

void MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	m_pPrimaryCommand = pPrimaryCommand;
	m_pRenderGraph->Run();
}

void MDeferredRenderProgram::RenderReady(MTaskNode* pTaskNode)
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
	UpdateRenderGroup(m_renderInfo);
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->UpdateShadowRenderGroup(m_renderInfo);
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
		ResizeForwardRenderPass(pViewport->GetSize(), pRenderSystem->GetDevice());

		if (m_pTransparentWork)
		{
			m_pTransparentWork->Resize(pViewport->GetSize());
			m_pTransparentWork->SetRenderTarget(GetOutputTexture(), m_forwardRenderPass.m_pDepthTexture);
		}
	}
}

void MDeferredRenderProgram::RenderGBuffer(MTaskNode* pTaskNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = m_renderInfo.pPrimaryRenderCommand;

	MViewport* pViewport = m_renderInfo.pViewport;


	if (MTexture* pShadowMap = GetShadowmapTexture())
	{
		pCommand->SetRenderToTextureBarrier({ pShadowMap });
	}

	pCommand->BeginRenderPass(&m_forwardRenderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));

	DrawStaticMesh(m_renderInfo, pCommand);

	pCommand->EndRenderPass();
}

void MDeferredRenderProgram::RenderLightning(MTaskNode* pTaskNode)
{

}

void MDeferredRenderProgram::RenderShadow(MTaskNode* pTaskNode)
{
	if (m_pShadowMapWork)
	{
		m_pShadowMapWork->RenderShadow(m_renderInfo);

		MTexture* pShadowMap = m_pShadowMapWork->GetShadowMap();

		m_frameParamSet.SetShadowMapTexture(pShadowMap);
	}
}

void MDeferredRenderProgram::RenderTransparent(MTaskNode* pTaskNode)
{
	if (m_pTransparentWork)
	{
		m_pTransparentWork->RenderDepthPeel(m_renderInfo);
		m_pTransparentWork->Render(m_renderInfo);
	}
}

MTexture* MDeferredRenderProgram::GetOutputTexture()
{
	return m_pFinalOutputTexture;
}

MTexture* MDeferredRenderProgram::GetShadowmapTexture()
{
	if (m_pShadowMapWork)
	{
		return m_pShadowMapWork->GetShadowMap();
	}

	return nullptr;
}

void MDeferredRenderProgram::DrawStaticMesh(MRenderInfo& info, MIRenderCommand* pCommand)
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

void MDeferredRenderProgram::OnCreated()
{
	Super::OnCreated();

	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pRenderGraph = pObjectSystem->CreateObject<MTaskGraph>();
	m_pShadowMapWork = pObjectSystem->CreateObject<MShadowMapRenderWork>();
	m_pTransparentWork = pObjectSystem->CreateObject<MTransparentRenderWork>();

	MTaskNode* pRenderReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pRenderReadyTask->SetThreadType(METhreadType::EAny);
	pRenderReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderReady, this));

 	MTaskNode* pRenderShadowTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Shadowmap");
 	pRenderShadowTask->SetThreadType(METhreadType::EAny);
 	pRenderShadowTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderShadow, this));

	MTaskNode* pRenderGBufferTask = m_pRenderGraph->AddNode<MTaskNode>("Render_GBuffer");
	pRenderGBufferTask->SetThreadType(METhreadType::EAny);
	pRenderGBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderGBuffer, this));

	MTaskNode* pRenderLightningTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Lightning");
	pRenderLightningTask->SetThreadType(METhreadType::EAny);
	pRenderLightningTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderLightning, this));

	MTaskNode* pRenderTransparentTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Transparent");
	pRenderTransparentTask->SetThreadType(METhreadType::EAny);
	pRenderTransparentTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MDeferredRenderProgram::RenderTransparent, this));

	/*
		RenderReady --> RenderShadowmap --> RenderForward --> RenderTransparent --> output				
	*/

 	pRenderReadyTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
	pRenderShadowTask->AppendOutput()->LinkTo(pRenderGBufferTask->AppendInput());
	pRenderGBufferTask->AppendOutput()->LinkTo(pRenderLightningTask->AppendInput());
	pRenderLightningTask->AppendOutput()->LinkTo(pRenderTransparentTask->AppendInput());

	InitializeFrameShaderParams();
	InitializeRenderPass();
}

void MDeferredRenderProgram::OnDelete()
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

void MDeferredRenderProgram::InitializeRenderPass()
{
	Vector2 v2Size = Vector2(512.0f, 512.0f);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const std::vector<MString> aTextureName = {
		"Base_Metallic",
		"Albedo_AmbientOcc",
		"Normal_Roughness",
		"Depth"
	};
	vkSetDebugUtilsObjectNameEXT

	for (const MString& strTextureName : aTextureName)
	{
		MTexture* pRenderTarget = MTexture::CreateRenderTarget();
		pRenderTarget->SetSize(v2Size);
		pRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
		m_gbufferRenderPass.m_vBackTextures.push_back(pRenderTarget);
		m_gbufferRenderPass.m_vBackDesc.push_back(MPassTargetDescription(true, MColor(0.0f, 0.0f, 0.0f, 1.0)));
	}

	m_gbufferRenderPass.m_pDepthTexture = MTexture::CreateShadowMap();
	m_gbufferRenderPass.m_pDepthTexture->SetSize(v2Size);
	m_gbufferRenderPass.m_pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_gbufferRenderPass.m_DepthDesc.bClearWhenRender = true;
	m_gbufferRenderPass.m_DepthDesc.cClearColor = MColor::White;

	m_gbufferRenderPass.GenerateBuffer(pRenderSystem->GetDevice());

	m_pFinalOutputTexture = pRenderTarget;
}

void MDeferredRenderProgram::ReleaseRenderPass()
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

void MDeferredRenderProgram::InitializeFrameShaderParams()
{
	m_frameParamSet.InitializeShaderParamSet(GetEngine());
}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_frameParamSet.ReleaseShaderParamSet(GetEngine());
}

void MDeferredRenderProgram::UpdateRenderGroup(MRenderInfo& info)
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
		if (!pMaterial)
			continue;

		MSceneComponent* pSceneComponent = meshComp.GetEntity()->GetComponent<MSceneComponent>();

		if (!pSceneComponent->GetVisibleRecursively())
			continue;

		const MBoundsAABB* pBounds = meshComp.GetBoundsAABB();

		if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum().ContainTest(*pBounds))
			continue;

		if (pMaterial->GetMaterialType() == MEMaterialType::EDepthPeel)
		{
			auto& meshes = m_renderInfo.m_tTransparentGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}
		else
		{
			auto& meshes = m_renderInfo.m_tMaterialGroupMesh[pMaterial];
			meshes.push_back(&meshComp);
		}


		pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
	}

	m_renderInfo.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_frameParamSet.UpdateShaderSharedParams(info);
	info.pFrameShaderParamSet = &m_frameParamSet;
}

void MDeferredRenderProgram::ResizeForwardRenderPass(const Vector2& v2Size, MIDevice* pDevice)
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
