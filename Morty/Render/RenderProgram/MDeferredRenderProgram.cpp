#include "MDeferredRenderProgram.h"

#include "Culling/MBoundingBoxCulling.h"
#include "Render/MRenderGlobal.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"
#include "Mesh/MMeshManager.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Utility/MBounds.h"
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
#include "Component/MRenderMeshComponent.h"

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
#include "RenderWork/MVoxelizerRenderWork.h"
#include "RenderWork/MVoxelDebugRenderWork.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Shadow/MShadowMeshManager.h"

#include "MeshRender/MSkyBoxRenderable.h"
#include "Batch/MMeshInstanceManager.h"
#include "MeshRender/MCullingResultRenderable.h"
#include "MeshRender/MIndirectIndexRenderable.h"
#include "MeshRender/MCullingResultSpecificMaterialRenderable.h"

#include "Culling/MCPUCameraFrustumCulling.h"
#include "Culling/MGPUCameraFrustumCulling.h"
#include "Manager/MAnimationManager.h"

#include "Culling/MBoundingBoxCulling.h"
#include "Manager/MEnvironmentManager.h"
#include "RenderWork/MVRSTextureRenderWork.h"
#include "TaskGraph/MMultiThreadTaskGraphWalker.h"
#include "Utility/MGlobal.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)


void MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	RenderSetup(pPrimaryCommand);
	RenderShadow();
	RenderVoxelizer();
	RenderGBuffer();
	RenderLightning();
	RenderForward();
	RenderVoxelizerDebug();
	RenderTransparent();
	RenderPostProcess();
	RenderVRS();
	RenderDebug();
}

void MDeferredRenderProgram::RenderSetup(MIRenderCommand* pPrimaryCommand)
{
	MViewport* pViewport = GetViewport();
	MEntity* pCameraEntity = pViewport->GetCamera();
	MScene* pScene = pViewport->GetScene();
	MEntity* pMainDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();

	m_renderInfo = MRenderInfo::CreateFromViewport(pViewport);
	m_renderInfo.nFrameIndex = m_nFrameIndex++;
	m_renderInfo.pPrimaryRenderCommand = pPrimaryCommand;


	//Shadow map Culling.
	auto* pShadowMapManager = pViewport->GetScene()->GetManager<MShadowMeshManager>();
	auto vShadowMaterialGroup = pShadowMapManager->GetAllShadowGroup();
	m_pShadowCulling->Get()->SetCamera(pCameraEntity);
	m_pShadowCulling->Get()->SetViewport(pViewport);
	m_pShadowCulling->Get()->SetDirectionalLight(pMainDirectionalLight);
	m_pShadowCulling->SetInput(vShadowMaterialGroup);


	//Scene Culling.
	auto* pMeshInstanceMeshManager = pScene->GetManager<MMeshInstanceManager>();
	const std::vector<MMaterialBatchGroup*> vMaterialGroup = pMeshInstanceMeshManager->GetAllMaterialGroup();

    const MCameraFrustum cameraFrustum = MRenderSystem::GetCameraFrustum(pViewport
		, pCameraEntity->GetComponent<MCameraComponent>()
		, pCameraSceneComponent
	);

	m_pCameraFrustumCulling->Get()->SetCommand(pPrimaryCommand);
	m_pCameraFrustumCulling->Get()->SetCameraFrustum(cameraFrustum);
	m_pCameraFrustumCulling->Get()->SetCameraPosition(pCameraSceneComponent->GetWorldPosition());
	m_pCameraFrustumCulling->SetInput(vMaterialGroup);

#if MORTY_VXGI_ENABLE
	uint32_t nClipmapIdx = m_renderInfo.nFrameIndex % MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM;
	GetRenderWork<MVoxelizerRenderWork>()->SetupVoxelSetting(m_renderInfo.m4CameraTransform.GetTranslation(), nClipmapIdx);
	auto voxelizerBounds = GetRenderWork<MVoxelizerRenderWork>()->GetVoxelizerBoundsAABB(nClipmapIdx);
	m_pVoxelizerCulling->SetInput(vMaterialGroup);
	//Voxelizer Culling.
	m_pVoxelizerCulling->Get()->SetBounds(voxelizerBounds);
#endif

	MMultiThreadTaskGraphWalker walker(GetEngine()->GetThreadPool());
	walker(m_pCullingTask.get());

	m_renderInfo.shadowRenderInfo = m_pShadowCulling->Get()->GetCascadedRenderInfo();

	//Update Shader Params.
	UpdateFrameParams(m_renderInfo);

	//Resize FrameBuffer.
	const Vector2i v2ViewportSize = pViewport->GetSize();
	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Resize(v2ViewportSize);
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

	InitializeTaskGraph();
	InitializeFrameShaderParams();
	InitializeRenderWork();
	InitializeRenderTarget();
}

void MDeferredRenderProgram::OnDelete()
{
	Super::OnDelete();

	ReleaseTaskGraph();
	ReleaseFrameShaderParams();
	ReleaseRenderWork();
	ReleaseRenderTarget();
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

#if MORTY_VXGI_ENABLE
	RegisterRenderWork<MVoxelizerRenderWork>();
	RegisterRenderWork<MVoxelDebugRenderWork>();
#endif


#if VRS_OPTIMIZE_ENABLE
	auto pDevice = GetEngine()->FindSystem<MRenderSystem>()->GetDevice();
	if (pDevice->GetDeviceFeatureSupport(MEDeviceFeature::EVariableRateShading))
	{
		RegisterRenderWork<MVRSTextureRenderWork>();
	}
#endif
}

void MDeferredRenderProgram::ReleaseRenderWork()
{
	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Release(GetEngine());
	}
	m_tRenderWork.clear();
}

void MDeferredRenderProgram::InitializeTaskGraph()
{
	m_pCullingTask = std::make_unique<MTaskGraph>();

	m_pShadowCulling = m_pCullingTask->AddNode<MCullingTaskNode<MCascadedShadowCulling>>(MStringId("Shadow Culling"));
	m_pShadowCulling->Initialize(GetEngine());

#if MORTY_VXGI_ENABLE
	m_pVoxelizerCulling = m_pCullingTask->AddNode<MCullingTaskNode<MBoundingBoxCulling>>(MStringId("Voxelizer Culling"));
	m_pVoxelizerCulling->Initialize(GetEngine());
#endif

#if GPU_CULLING_ENABLE
	m_pCameraFrustumCulling = m_pCullingTask->AddNode<MCullingTaskNode<MGPUCameraFrustumCulling>>(MStringId("Gpu Camera Culling"));
	m_pCameraFrustumCulling->Initialize(GetEngine());
#else
	m_pCameraFrustumCulling = m_pCullingTask->AddNode<MCullingTaskNode<MCPUCameraFrustumCulling>>(MStringId("Cpu Camera Culling"));
	m_pCameraFrustumCulling->Initialize(GetEngine());
#endif
}

void MDeferredRenderProgram::ReleaseTaskGraph()
{
	m_pShadowCulling->Release();
	m_pShadowCulling = nullptr;

#if MORTY_VXGI_ENABLE
	m_pVoxelizerCulling->Release();
	m_pVoxelizerCulling = nullptr;
#endif

	m_pCameraFrustumCulling->Release();
	m_pCameraFrustumCulling = nullptr;

	m_pCullingTask = nullptr;
}

void MDeferredRenderProgram::InitializeRenderTarget()
{
	const Vector2i n2Size = Vector2i(512, 512);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	const std::vector<std::pair<MString, MPassTargetDescription>> vTextureDesc = {
		{"f3Albedo_fMetallic", {true, MColor::Black_T} },
		{"u_mat_f3Normal_fRoughness", {true, MColor::Black_T} },
		{"u_mat_f3Position_fAmbientOcc", {true, MColor::Black_T} }
	};

	MRenderTarget shadingRate;

	if (auto pVRSRenderWork = GetRenderWork<MVRSTextureRenderWork>())
	{
		shadingRate.pTexture = pVRSRenderWork->GetVRSTexture();
#if MORTY_DEBUG
		m_vRenderTargets.push_back(shadingRate.pTexture);
#endif
	}

	std::vector<MRenderTarget> vBackTextures;
	for (auto desc : vTextureDesc)
	{
		std::shared_ptr<MTexture> pBackTexture = MTexture::CreateRenderTargetGBuffer();
		pBackTexture->SetName(desc.first);
		pBackTexture->SetSize(n2Size);
		pBackTexture->GenerateBuffer(pRenderSystem->GetDevice());

		vBackTextures.push_back({ pBackTexture, {true, MColor::Black_T} });
		m_vRenderTargets.push_back(pBackTexture);
	}

	std::shared_ptr<MTexture> pDepthTexture = MTexture::CreateShadowMap();
	pDepthTexture->SetName("Depth Texture");
	pDepthTexture->SetSize(n2Size);
	pDepthTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pDepthTexture);


	std::shared_ptr<MTexture> pShadowTexture = MTexture::CreateShadowMapArray(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
	pShadowTexture->SetSize(Vector2i(MRenderGlobal::SHADOW_TEXTURE_SIZE, MRenderGlobal::SHADOW_TEXTURE_SIZE));
	pShadowTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pShadowTexture);


	std::shared_ptr<MTexture> pLightningRenderTarget = MTexture::CreateRenderTarget(METextureLayout::ERGBA_FLOAT_16);
	pLightningRenderTarget->SetName("Lightning Output");
	pLightningRenderTarget->SetSize(n2Size);
	pLightningRenderTarget->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pLightningRenderTarget);


	std::shared_ptr<MTexture> pPostProcessOutput = MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8);
	pPostProcessOutput->SetName("Post Process Output");
	pPostProcessOutput->SetSize(n2Size);
	pPostProcessOutput->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pPostProcessOutput);


	GetRenderWork<MShadowMapRenderWork>()->SetRenderTarget({
	    {},
		{ pShadowTexture, { true, MColor::White }},
		{}
	});

	GetRenderWork<MGBufferRenderWork>()->SetRenderTarget(
	{
	    vBackTextures,
	    { pDepthTexture,{true, MColor::Black_T} },
		shadingRate
	});
	GetRenderWork<MDeferredLightingRenderWork>()->SetRenderTarget({
	    {{pLightningRenderTarget, {true, MColor::Black_T }} },
	    {},
		shadingRate
	});
	GetRenderWork<MForwardRenderWork>()->SetRenderTarget({
		{ {pLightningRenderTarget, {false, MColor::Black_T }} },
		{ pDepthTexture, {false, MColor::Black_T} },
		shadingRate
	});

#if MORTY_VXGI_ENABLE
	std::shared_ptr<MTexture> pVoxelDebugTexture = MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8);
	pVoxelDebugTexture->SetName("Voxel Debug Texture");
	pVoxelDebugTexture->SetSize(n2Size);
	pVoxelDebugTexture->GenerateBuffer(pRenderSystem->GetDevice());
	m_vRenderTargets.push_back(pVoxelDebugTexture);

	GetRenderWork<MVoxelDebugRenderWork>()->SetRenderTarget({
		{ {pVoxelDebugTexture, {true, MColor::Black_T }} },
		{ pDepthTexture, {false, MColor::Black_T} },
	    {}
	});
#endif

	GetRenderWork<MPostProcessRenderWork>()->SetRenderTarget(
	    { pPostProcessOutput, {true, MColor::Black_T } }
	);

	GetRenderWork<MDebugRenderWork>()->SetRenderTarget({
		{ {pPostProcessOutput, {false, MColor::Black_T }} },
		{ pDepthTexture, {false, MColor::Black_T} },
		{}
	});

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
	m_pFramePropertyAdapter = std::make_shared<MFrameShaderPropertyBlock>();
	m_pFramePropertyAdapter->Initialize(GetEngine());
	m_pFramePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MFramePropertyDecorator>());
	m_pFramePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MLightPropertyDecorator>());
	m_pFramePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MAnimationPropertyDecorator>());


}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_pFramePropertyAdapter->Release(GetEngine());
	m_pFramePropertyAdapter = nullptr;

}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_pFramePropertyAdapter->UpdateShaderSharedParams(info);
}

void MDeferredRenderProgram::RenderGBuffer()
{
	if (!GetRenderWork<MGBufferRenderWork>())
	{
		MORTY_ASSERT(GetRenderWork<MGBufferRenderWork>());
		return;
	}

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	//Camera frustum culling.

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
	    m_pFramePropertyAdapter,
	});

	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDeferred));
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling->Get());

	GetRenderWork<MGBufferRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
	});
}

void MDeferredRenderProgram::RenderLightning()
{
	MORTY_ASSERT(GetRenderWork<MDeferredLightingRenderWork>());

	GetRenderWork<MDeferredLightingRenderWork>()->SetGBuffer(GetRenderWork<MGBufferRenderWork>()->CreateGBuffer());
	GetRenderWork<MDeferredLightingRenderWork>()->SetShadowMap(GetRenderWork<MShadowMapRenderWork>()->GetShadowMap());
	GetRenderWork<MDeferredLightingRenderWork>()->SetFrameProperty(m_pFramePropertyAdapter);

	GetRenderWork<MDeferredLightingRenderWork>()->Render(m_renderInfo);
}

void MDeferredRenderProgram::RenderVoxelizer()
{
	if (!GetRenderWork<MVoxelizerRenderWork>())
	{
		return;
	}
	
	auto pVoxelizerWork = GetRenderWork<MVoxelizerRenderWork>();

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MCullingResultSpecificMaterialRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
	});
	indirectMesh.SetInstanceCulling(m_pVoxelizerCulling->Get());
	indirectMesh.SetMaterial(pVoxelizerWork->GetVoxelizerMaterial());

	std::unordered_map<MStringId, bool> tVoxelizerDefined = {
		{ MRenderGlobal::SHADER_SKELETON_ENABLE, false }
	};

	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialMacroDefineFilter>(tVoxelizerDefined));

	pVoxelizerWork->Render(m_renderInfo, {
		&indirectMesh,
	});


}

void MDeferredRenderProgram::RenderShadow()
{
	MORTY_ASSERT(GetRenderWork<MShadowMapRenderWork>());
	
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
	});
	indirectMesh.SetInstanceCulling(m_pShadowCulling->Get());

    GetRenderWork<MShadowMapRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
	});

}

void MDeferredRenderProgram::RenderForward()
{
	if (!GetRenderWork<MForwardRenderWork>())
	{
		MORTY_ASSERT(GetRenderWork<MForwardRenderWork>());
		return;
	}

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
		});
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDefault));
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling->Get());

	const MEnvironmentManager* pEnvironmentManager = m_renderInfo.pScene->GetManager<MEnvironmentManager>();
	const auto pMaterial = pEnvironmentManager->GetMaterial();

	MSkyBoxRenderable skyBox;
	skyBox.SetMesh(pMeshManager->GetSkyBox());
	skyBox.SetMaterial(pMaterial);
	skyBox.SetPropertyBlockAdapter({ m_pFramePropertyAdapter });

	GetRenderWork<MForwardRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
		&skyBox,
		});

}


void MDeferredRenderProgram::RenderVoxelizerDebug()
{
	auto pVoxelizerWork = GetRenderWork<MVoxelizerRenderWork>();
	auto pVoxelDebugWork = GetRenderWork<MVoxelDebugRenderWork>();
	if (!pVoxelizerWork)
	{
		return;
	}

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MIndirectIndexRenderable debugRender;
	debugRender.SetMaterial(pVoxelDebugWork->GetVoxelDebugMaterial());
	debugRender.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter
		});
	debugRender.SetIndirectIndexBuffer(pVoxelDebugWork->GetVoxelDebugBuffer());
	debugRender.SetMeshBuffer(pMeshManager->GetMeshBuffer());

	pVoxelDebugWork->Render(m_renderInfo, pVoxelizerWork->GetVoxelSetting(), pVoxelizerWork->GetVoxelTableBuffer(), {
	   &debugRender,
    });
}

void MDeferredRenderProgram::RenderTransparent()
{
	if (!GetRenderWork<MTransparentRenderWork>())
	{
		return;
	}

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
		});
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDepthPeel));
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling->Get());

	GetRenderWork<MTransparentRenderWork>()->Render(m_renderInfo, { &indirectMesh });
}

void MDeferredRenderProgram::RenderPostProcess()
{
	MORTY_ASSERT(GetRenderWork<MPostProcessRenderWork>());
    GetRenderWork<MPostProcessRenderWork>()->Render(m_renderInfo);
}

void MDeferredRenderProgram::RenderDebug()
{
	MORTY_ASSERT(GetRenderWork<MDebugRenderWork>());

	//Current viewport.
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();


	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({ m_pFramePropertyAdapter });
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling->Get());

	GetRenderWork<MDebugRenderWork>()->Render(m_renderInfo, {
		&indirectMesh
		});


}

void MDeferredRenderProgram::RenderVRS()
{
	if (!GetRenderWork<MVRSTextureRenderWork>() || !GetRenderWork<MPostProcessRenderWork>())
	{
		return;
	}

	const auto pEdgeTexture = GetRenderWork<MPostProcessRenderWork>()->GetOutput(MRenderGlobal::POSTPROCESS_EDGE_DETECTION);

	GetRenderWork<MVRSTextureRenderWork>()->Render(m_renderInfo, pEdgeTexture);
}
