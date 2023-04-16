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
#include "RenderWork/MGPUCullingRenderWork.h"
#include "RenderWork/MTransparentRenderWork.h"
#include "RenderWork/MPostProcessRenderWork.h"
#include "RenderWork/MEnvironmentMapRenderWork.h"
#include "Component/MCameraComponent.h"
#include "Shadow/MShadowMapManager.h"
#include "MergeInstancing/MMergeInstancingSubSystem.h"
#include "Render/MVertex.h"

#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"

#include "Mesh/MMeshManager.h"

#include "MeshRender/MSkyBoxRender.h"
#include "MeshRender/MStaticMeshRender.h"
#include "MergeInstancing/MRenderableMeshManager.h"
#include "MeshRender/MGPUDrivenRender.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)


class CameraFrustumCulling : public IRenderableFilter
{
public:
	CameraFrustumCulling(Matrix4 cameraInvProj)
	{
		m_cameraFrustum.UpdateFromCameraInvProj(cameraInvProj);
	}

	bool Filter(const MRenderableMeshInstance* instance) const override
	{
		const MBoundsAABB& bounds = instance->boundsWithTransform;
		if (MCameraFrustum::EOUTSIDE == m_cameraFrustum.ContainTest(bounds))
		{
			return false;
		}

		return true;
	}

private:

	MCameraFrustum m_cameraFrustum;
};


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
	m_renderInfo.pCameraEntity = pViewport->GetCamera();

	auto shadowSceneData = MShadowMapUtil::CascadedSplitCameraFrustum(m_renderInfo.pViewport);
	m_renderInfo.shadowRenderInfo = MShadowMapUtil::CalculateRenderData(
		m_renderInfo.pViewport,
		m_renderInfo.pCameraEntity,
		shadowSceneData
	);

	//Culling

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
	RegisterRenderWork<MGPUCullingRenderWork>();
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

	MTaskNode* pRenderCullingTask = m_pRenderGraph->AddNode<MTaskNode>("Render_Culling");
	pRenderCullingTask->SetThreadType(METhreadType::EAny);
	pRenderCullingTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MDeferredRenderProgram::RenderCulling, this));

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
		RenderReady --> RenderCulling --> RenderShadowmap --> pRenderEnvironmentTask --> RenderGBuffer --> RenderLightning --> RenderForward --> RenderTransparent --> RenderDebug --> output
	*/

	pRenderReadyTask->AppendOutput()->LinkTo(pRenderCullingTask->AppendInput());
	pRenderCullingTask->AppendOutput()->LinkTo(pRenderShadowTask->AppendInput());
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

	GetRenderWork<MDebugRenderWork>()->SetRenderTarget(
		{ {pLightningRenderTarget, {false, true, MColor::Black_T }} },
		{ pDepthTexture, {false, true, MColor::Black_T} });

	GetRenderWork<MPostProcessRenderWork>()->SetRenderTarget(
		{ {pPostProcessOutput, {true, false, MColor::Black_T }} });

	m_pFinalOutputTexture = pPostProcessOutput;


	GetRenderWork<MPostProcessRenderWork>()->SetInputTexture(GetRenderWork<MDeferredLightingRenderWork>()->CreateOutput());
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
}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_pFramePropertyAdapter->Release(GetEngine());
	m_pFramePropertyAdapter = nullptr;

	m_pShadowPropertyAdapter->Release(GetEngine());
	m_pShadowPropertyAdapter = nullptr;
}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_pFramePropertyAdapter->UpdateShaderSharedParams(info);
	m_pShadowPropertyAdapter->UpdateShaderSharedParams(info);
}

void MDeferredRenderProgram::RenderCulling(MTaskNode* pTaskNode)
{
	if (GetRenderWork<MGPUCullingRenderWork>() && m_bGPUCullingUpdate)
	{
		GetRenderWork<MGPUCullingRenderWork>()->CollectCullingGroup(m_renderInfo);
		m_bGPUCullingUpdate = false;
	}

	GetRenderWork<MGPUCullingRenderWork>()->UpdateCameraFrustum(m_renderInfo);
	GetRenderWork<MGPUCullingRenderWork>()->DispatchCullingJob(m_renderInfo);
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
	auto* pRenderableMeshManager = pScene->GetManager<MRenderableMeshManager>();
	//Camera frustum culling.

	//Render static mesh.
	MStaticMeshRender staticMesh;
	staticMesh.SetScene(pScene);
	staticMesh.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);
	auto pCameraFrustumCulling = std::make_shared<CameraFrustumCulling>(pViewport->GetCameraInverseProjection());
	staticMesh.SetRenderableFilter(pCameraFrustumCulling);
	std::vector<MRenderableMaterialGroup*> vRenderableGroup = pRenderableMeshManager->FindGroupFromMaterialType(MEMaterialType::EDeferred);
	staticMesh.SetRenderableMaterialGroup(vRenderableGroup);

	GetRenderWork<MGBufferRenderWork>()->Render(m_renderInfo, {
		&staticMesh,
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
	
	if (m_pShadowPropertyAdapter->GetCount() == 0)
	{
		return;
	}

	//Current viewport.
	MScene* pScene = m_renderInfo.pViewport->GetScene();
	//culling
	MComputeDispatcherRender shadowCulling;
	shadowCulling.SetComputeDispatcher(m_pShadowPropertyAdapter);
	shadowCulling.Render(m_renderInfo.pPrimaryRenderCommand);

	//draw merge instancing
	MGPUDrivenRender mergeInstancing;
	mergeInstancing.SetScene(pScene);
	mergeInstancing.SetDrawIndirect(m_pShadowPropertyAdapter);
	mergeInstancing.SetFramePropertyBlockAdapter(m_pShadowPropertyAdapter);

    GetRenderWork<MShadowMapRenderWork>()->Render(m_renderInfo, {
		&mergeInstancing,
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
	//Camera frustum culling.

	//Render static mesh.
	MStaticMeshRender staticMesh;
	staticMesh.SetScene(pScene);
	staticMesh.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);
	auto pCameraFrustumCulling = std::make_shared<CameraFrustumCulling>(pViewport->GetCameraInverseProjection());
	staticMesh.SetRenderableFilter(pCameraFrustumCulling);
	std::vector<MRenderableMaterialGroup*> vRenderableGroup = pRenderableMeshManager->FindGroupFromMaterialType(MEMaterialType::EDefault);
	staticMesh.SetRenderableMaterialGroup(vRenderableGroup);

	MSkyBoxRender skyBox;
	skyBox.SetScene(pScene);
	skyBox.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);

	GetRenderWork<MForwardRenderWork>()->Render(m_renderInfo, {
		&staticMesh,
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
	GetRenderWork<MDebugRenderWork>()->Render(m_renderInfo);
}
