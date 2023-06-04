#include "MDeferredRenderProgram.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Utility/MFunction.h"
#include "Model/MSkeleton.h"
#include "TaskGraph/MTaskNode.h"
#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"
#include "TaskGraph/MTaskNodeOutput.h"

#include "Basic/MCameraFrustum.h"
#include "Render/MRenderCommand.h"

#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "RenderWork/MGBufferRenderWork.h"
#include "RenderWork/MDeferredLightingRenderWork.h"
#include "RenderWork/MShadowMapRenderWork.h"
#include "RenderWork/MForwardRenderWork.h"
#include "RenderWork/MDebugRenderWork.h"
#include "RenderWork/MTransparentRenderWork.h"
#include "RenderWork/MPostProcessRenderWork.h"
#include "RenderWork/MEnvironmentMapRenderWork.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Shadow/MShadowMapManager.h"
#include "Render/MVertex.h"

#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"

#include "Mesh/MMeshManager.h"

#include "MeshRender/MSkyBoxRender.h"
#include "MeshRender/MMaterialGroupRenderable.h"
#include "MergeInstancing/MRenderableMeshManager.h"
#include "MeshRender/MIndexdIndirectRenderable.h"

#include "Culling/MSceneCulling.h"
#include "Culling/MSceneGPUCulling.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)


#define GPU_CULLING_ENABLE false


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
	MEntity* pCameraEntity = pViewport->GetCamera();
	MScene* pScene = pViewport->GetScene();
	MEntity* pMainDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
	MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();

	m_renderInfo = MRenderInfo();
	m_renderInfo.nFrameIndex = m_nFrameIndex++;
	m_renderInfo.pViewport = pViewport;
	m_renderInfo.pPrimaryRenderCommand = m_pPrimaryCommand;
	m_renderInfo.pCameraEntity = pCameraEntity;
	m_renderInfo.pDirectionalLightEntity = pMainDirectionalLight;
	m_renderInfo.m4CameraInverseProjection = pRenderSystem->GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent);
	m_renderInfo.cameraFrustum.UpdateFromCameraInvProj(m_renderInfo.m4CameraInverseProjection);

	//Update Camera frustum for culling.
	m_pCameraFrustumCulling->UpdateCameraFrustum(m_renderInfo.m4CameraInverseProjection);

	//Shadow map Culling.
	auto* pShadowMapManager = pViewport->GetScene()->GetManager<MShadowMapManager>();
	MRenderableMaterialGroup* pShadowMaterialGroup = pShadowMapManager->GetStaticShadowGroup();
	m_pShadowCulling->SetViewport(pViewport);
	m_pShadowCulling->SetCamera(pCameraEntity);
	m_pShadowCulling->SetDirectionalLight(pMainDirectionalLight);
	m_pShadowCulling->Culling({  pShadowMaterialGroup });

	//CPU Culling.
	auto* pRenderableMeshManager = pScene->GetManager<MRenderableMeshManager>();
	const std::vector<MRenderableMaterialGroup*> vMaterialGroup = pRenderableMeshManager->GetAllMaterialGroup();

#if GPU_CULLING_ENABLE
	//GPU Culling.
	MCameraFrustum cameraFrustum = MRenderSystem::GetCameraFrustum(pViewport
		, pCameraEntity->GetComponent<MCameraComponent>()
		, pCameraSceneComponent
	);
	m_pGpuCulling->SetCommand(m_pPrimaryCommand);
	m_pGpuCulling->SetCameraFrustum(cameraFrustum);
	m_pGpuCulling->SetCameraPosition(pCameraSceneComponent->GetWorldPosition());
	m_pGpuCulling->Culling(vMaterialGroup);

#else
	m_pCpuCulling->Culling(vMaterialGroup);
#endif

	m_renderInfo.shadowRenderInfo = m_pShadowCulling->GetCascadedRenderData();

	//Update Shader Params.
	UpdateFrameParams(m_renderInfo);

	//Resize FrameBuffer.
	const Vector2 v2Size = pViewport->GetSize();
	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Resize(v2Size);
	}
}

std::shared_ptr<MTexture> MDeferredRenderProgram::GetOutputTexture()
{
	return m_pFinalOutputTexture;
}

std::vector<std::shared_ptr<MTexture>> MDeferredRenderProgram::GetOutputTextures()
{
	return m_vRenderTargets;
}

void MDeferredRenderProgram::OnCreated()
{
	Super::OnCreated();
	InitializeRenderGraph();

	InitializeRenderWork();
	InitializeFrameShaderParams();
	InitializeRenderTarget();
}

void MDeferredRenderProgram::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	Super::OnDelete();

	ReleaseRenderWork();
	ReleaseRenderTarget();
	ReleaseFrameShaderParams();

	m_pRenderGraph->DeleteLater();
	m_pRenderGraph = nullptr;
}

void MDeferredRenderProgram::InitializeRenderWork()
{
	RegisterRenderWork<MGBufferRenderWork>();
	RegisterRenderWork<MDeferredLightingRenderWork>();
	RegisterRenderWork<MShadowMapRenderWork>();
	RegisterRenderWork<MForwardRenderWork>();
	RegisterRenderWork<MDebugRenderWork>();
	//RegisterRenderWork<MTransparentRenderWork>();
	RegisterRenderWork<MPostProcessRenderWork>();

	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Initialize(GetEngine());
	}
}

void MDeferredRenderProgram::ReleaseRenderWork()
{
	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Release(GetEngine());
	}
	m_tRenderWork.clear();
}

void MDeferredRenderProgram::InitializeRenderGraph()
{
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pRenderGraph = pObjectSystem->CreateObject<MTaskGraph>();

	MTaskNode* pRenderReadyTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Ready");
	pRenderReadyTask->SetThreadType(METhreadType::EAny);
	pRenderReadyTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderReady, this));

	MTaskNode* pRenderShadowTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Shadowmap");
	pRenderShadowTask->SetThreadType(METhreadType::EAny);
	pRenderShadowTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderShadow, this));

	MTaskNode* pRenderGBufferTask = m_pRenderGraph->AddNode<MTaskNode>("Render_GBuffer");
	pRenderGBufferTask->SetThreadType(METhreadType::EAny);
	pRenderGBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderGBuffer, this));

	MTaskNode* pRenderLightningTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Lightning");
	pRenderLightningTask->SetThreadType(METhreadType::EAny);
	pRenderLightningTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderLightning, this));

	MTaskNode* pRenderForwardTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Forward");
	pRenderForwardTask->SetThreadType(METhreadType::EAny);
	pRenderForwardTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderForward, this));

	MTaskNode* pRenderTransparentTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Transparent");
	pRenderTransparentTask->SetThreadType(METhreadType::EAny);
	pRenderTransparentTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderTransparent, this));

	MTaskNode* pRenderPostProcessTask = m_pRenderGraph->AddNode<MTaskNode>("Render_PostProcess");
	pRenderPostProcessTask->SetThreadType(METhreadType::EAny);
	pRenderPostProcessTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderPostProcess, this));

	MTaskNode* pRenderDebugTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Debug");
	pRenderDebugTask->SetThreadType(METhreadType::EAny);
	pRenderDebugTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderDebug, this));


	/*
		RenderReady --> RenderCulling --> RenderShadowmap --> RenderGBuffer --> RenderLightning --> RenderForward --> RenderTransparent --> RenderDebug --> output
	*/

	pRenderReadyTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
	pRenderShadowTask->AppendOutput()->LinkTo(pRenderGBufferTask->AppendInput());
	pRenderGBufferTask->AppendOutput()->LinkTo(pRenderLightningTask->AppendInput());
	pRenderLightningTask->AppendOutput()->LinkTo(pRenderForwardTask->AppendInput());
	pRenderForwardTask->AppendOutput()->LinkTo(pRenderTransparentTask->AppendInput());
	pRenderTransparentTask->AppendOutput()->LinkTo(pRenderPostProcessTask->AppendInput());
	pRenderPostProcessTask->AppendOutput()->LinkTo(pRenderDebugTask->AppendInput());
}

void MDeferredRenderProgram::InitializeRenderTarget()
{
	Vector2 v2Size = Vector2(512.0f, 512.0f);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const std::vector<std::pair<MString, MPassTargetDescription>> vTextureDesc = {
		{"f3Albedo_fMetallic", {true, MColor::Black_T} },
		{"u_mat_f3Normal_fRoughness", {true, MColor::Black_T} },
		{"u_mat_f3Position_fAmbientOcc", {true, MColor::Black_T} }
	};


	std::vector<MRenderTarget> vBackTextures;
	for (auto desc : vTextureDesc)
	{
		std::shared_ptr<MTexture> pBackTexture = MTexture::CreateRenderTargetGBuffer();
		pBackTexture->SetName(desc.first);
		pBackTexture->SetSize(v2Size);
		pBackTexture->GenerateBuffer(pRenderSystem->GetDevice());

		vBackTextures.push_back({ pBackTexture, {true, false, MColor::Black_T} });
		m_vRenderTargets.push_back(pBackTexture);
	}

	std::shared_ptr<MTexture> pDepthTexture = MTexture::CreateShadowMap();
	pDepthTexture->SetSize(v2Size);
	pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pDepthTexture);


	std::shared_ptr<MTexture> pShadowTexture = MTexture::CreateShadowMapArray(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
	pShadowTexture->SetSize(Vector2(MRenderGlobal::SHADOW_TEXTURE_SIZE, MRenderGlobal::SHADOW_TEXTURE_SIZE));
	pShadowTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pShadowTexture);


	std::shared_ptr<MTexture> pLightningRenderTarget = MTexture::CreateRenderTarget(METextureLayout::ERGBA_FLOAT_16);
	pLightningRenderTarget->SetName("Lightning Output");
	pLightningRenderTarget->SetSize(v2Size);
	pLightningRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pLightningRenderTarget);


	std::shared_ptr<MTexture> pPostProcessOutput = MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8);
	pPostProcessOutput->SetName("Post Process Output");
	pPostProcessOutput->SetSize(v2Size);
	pPostProcessOutput->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pPostProcessOutput);


	GetRenderWork<MShadowMapRenderWork>()->SetRenderTarget({}, { pShadowTexture, { true, false, MColor::White }});
	m_pFramePropertyAdapter->SetShadowMapTexture(pShadowTexture);

	
	GetRenderWork<MGBufferRenderWork>()->SetRenderTarget(vBackTextures, { pDepthTexture, {true, false, MColor::Black_T} });
	GetRenderWork<MDeferredLightingRenderWork>()->SetRenderTarget({{pLightningRenderTarget, {true, false, MColor::Black_T }} });
	GetRenderWork<MForwardRenderWork>()->SetRenderTarget(
		{ {pLightningRenderTarget, {false, true, MColor::Black_T }} },
		{ pDepthTexture, {false, true, MColor::Black_T} });

	GetRenderWork<MPostProcessRenderWork>()->SetRenderTarget(
		{ {pPostProcessOutput, {true, false, MColor::Black_T }} });

	GetRenderWork<MDebugRenderWork>()->SetRenderTarget(
		{ {pPostProcessOutput, {false, true, MColor::Black_T }} },
		{ pDepthTexture, {false, true, MColor::Black_T} });

	m_pFinalOutputTexture = pPostProcessOutput;


	GetRenderWork<MPostProcessRenderWork>()->SetInputTexture(GetRenderWork<MForwardRenderWork>()->CreateOutput());
}

void MDeferredRenderProgram::ReleaseRenderTarget()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	for (auto pTexture : m_vRenderTargets)
	{
		pTexture->DestroyBuffer(pRenderSystem->GetDevice());
	}

	m_vRenderTargets.clear();
}

void MDeferredRenderProgram::InitializeFrameShaderParams()
{
	m_pFramePropertyAdapter = std::make_shared<MForwardRenderShaderPropertyBlock>();
	m_pFramePropertyAdapter->Initialize(GetEngine());

	m_pShadowPropertyAdapter = std::make_shared<MShadowMapShaderPropertyBlock>();
	m_pShadowPropertyAdapter->Initialize(GetEngine());

	m_pCameraFrustumCulling = std::make_shared<CameraFrustumCulling>();

	m_pShadowCulling = std::make_shared<MCascadedShadowCulling>();
	m_pShadowCulling->Initialize(GetEngine());

	m_pCpuCulling = std::make_shared<MSceneCulling>();
	m_pCpuCulling->Initialize(GetEngine());
	m_pCpuCulling->AddFilter(m_pCameraFrustumCulling);

	m_pGpuCulling = std::make_shared<MSceneGPUCulling>();
	m_pGpuCulling->Initialize(GetEngine());
}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_pFramePropertyAdapter->Release(GetEngine());
	m_pFramePropertyAdapter = nullptr;

	m_pShadowPropertyAdapter->Release(GetEngine());
	m_pShadowPropertyAdapter = nullptr;

	m_pShadowCulling->Release();
	m_pShadowCulling = nullptr;

	m_pCpuCulling->Release();
	m_pCpuCulling = nullptr;

	m_pGpuCulling->Release();
	m_pGpuCulling = nullptr;
}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_pFramePropertyAdapter->UpdateShaderSharedParams(info);
	m_pShadowPropertyAdapter->UpdateShaderSharedParams(info);
}

void MDeferredRenderProgram::RenderGBuffer(MTaskNode* pTaskNode)
{
	if (!GetRenderWork<MGBufferRenderWork>())
	{
		MORTY_ASSERT(GetRenderWork<MGBufferRenderWork>());
		return;
	}

	//Current viewport.
	MViewport* pViewport = m_renderInfo.pViewport;
	MScene* pScene = pViewport->GetScene();
	//Camera frustum culling.

	//Render static mesh.
	MIndexdIndirectRenderable indirectMesh;
	indirectMesh.SetScene(pScene);
	indirectMesh.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDeferred));

#if GPU_CULLING_ENABLE
	indirectMesh.SetInstanceCulling(m_pGpuCulling);
#else
	indirectMesh.SetInstanceCulling(m_pCpuCulling);
#endif

	GetRenderWork<MGBufferRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
	});
}
void MDeferredRenderProgram::RenderLightning(MTaskNode* pTaskNode)
{
	MORTY_ASSERT(GetRenderWork<MDeferredLightingRenderWork>());

	GetRenderWork<MDeferredLightingRenderWork>()->SetGBuffer(GetRenderWork<MGBufferRenderWork>()->CreateGBuffer());
	GetRenderWork<MDeferredLightingRenderWork>()->SetShadowMap(GetRenderWork<MShadowMapRenderWork>()->GetShadowMap());
	GetRenderWork<MDeferredLightingRenderWork>()->SetFrameProperty(m_pFramePropertyAdapter);

	GetRenderWork<MDeferredLightingRenderWork>()->Render(m_renderInfo);
}

void MDeferredRenderProgram::RenderShadow(MTaskNode* pTaskNode)
{
	MORTY_ASSERT(GetRenderWork<MShadowMapRenderWork>());
	
	//Current viewport.
	MViewport* pViewport = m_renderInfo.pViewport;
	MScene* pScene = pViewport->GetScene();
	auto* pShadowMapManager = pScene->GetManager<MShadowMapManager>();

	MIndexdIndirectRenderable indirectMesh;
	indirectMesh.SetScene(pScene);
	indirectMesh.SetFramePropertyBlockAdapter(m_pShadowPropertyAdapter);
	indirectMesh.SetInstanceCulling(m_pShadowCulling);

    GetRenderWork<MShadowMapRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
	});

}

void MDeferredRenderProgram::RenderForward(MTaskNode* pTaskNode)
{
	if (!GetRenderWork<MForwardRenderWork>())
	{
		MORTY_ASSERT(GetRenderWork<MForwardRenderWork>());
		return;
	}

	//Current viewport.
	MViewport* pViewport = m_renderInfo.pViewport;
	MScene* pScene = pViewport->GetScene();
	auto* pRenderableMeshManager = pScene->GetManager<MRenderableMeshManager>();
    
	//Render static mesh.
	MIndexdIndirectRenderable indirectMesh;
	indirectMesh.SetScene(pScene);
	indirectMesh.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDefault));
#if GPU_CULLING_ENABLE
	indirectMesh.SetInstanceCulling(m_pGpuCulling);
#else
	indirectMesh.SetInstanceCulling(m_pCpuCulling);
#endif


	MSkyBoxRender skyBox;
	skyBox.SetScene(pScene);
	skyBox.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);


	GetRenderWork<MForwardRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
		&skyBox,
		});
}

void MDeferredRenderProgram::RenderTransparent(MTaskNode* pTaskNode)
{
	if (GetRenderWork<MTransparentRenderWork>())
	{
		GetRenderWork<MTransparentRenderWork>()->Render(m_renderInfo);
	}
}

void MDeferredRenderProgram::RenderPostProcess(MTaskNode* pTaskNode)
{
	MORTY_ASSERT(GetRenderWork<MPostProcessRenderWork>());
    GetRenderWork<MPostProcessRenderWork>()->Render(m_renderInfo);
}

void MDeferredRenderProgram::RenderDebug(MTaskNode* pTaskNode)
{
	MORTY_ASSERT(GetRenderWork<MDebugRenderWork>());

	//Current viewport.
	MViewport* pViewport = m_renderInfo.pViewport;
	MScene* pScene = pViewport->GetScene();

	//Render static mesh.
	MIndexdIndirectRenderable indirectMesh;
	indirectMesh.SetScene(pScene);
	indirectMesh.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
#if GPU_CULLING_ENABLE
	indirectMesh.SetInstanceCulling(m_pGpuCulling);
#else
	indirectMesh.SetInstanceCulling(m_pCpuCulling);
#endif

	GetRenderWork<MDebugRenderWork>()->Render(m_renderInfo, {
		&indirectMesh
		});
}
