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
#include "Utility/MGlobal.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)


#define GPU_CULLING_ENABLE false

void MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	RenderSetup(pPrimaryCommand);
	RenderShadow();
	RenderGBuffer();
	RenderLightning();
	RenderForward();
	RenderVoxelizer();
	RenderTransparent();
	RenderPostProcess();
	RenderDebug();
}

void MDeferredRenderProgram::RenderSetup(MIRenderCommand* pPrimaryCommand)
{
	MEngine* pEngine = GetEngine();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	MViewport* pViewport = GetViewport();
	MEntity* pCameraEntity = pViewport->GetCamera();
	MScene* pScene = pViewport->GetScene();
	MEntity* pMainDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
	MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
	MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();

	m_renderInfo = MRenderInfo();
	m_renderInfo.nFrameIndex = m_nFrameIndex++;
	m_renderInfo.pViewport = pViewport;
	m_renderInfo.pPrimaryRenderCommand = pPrimaryCommand;
	m_renderInfo.pCameraEntity = pCameraEntity;
	m_renderInfo.pDirectionalLightEntity = pMainDirectionalLight;
	m_renderInfo.m4CameraInverseProjection = pRenderSystem->GetCameraInverseProjection(pViewport, pCameraComponent, pCameraSceneComponent);
	m_renderInfo.cameraFrustum.UpdateFromCameraInvProj(m_renderInfo.m4CameraInverseProjection);
	
	//Shadow map Culling.
	auto* pShadowMapManager = pViewport->GetScene()->GetManager<MShadowMeshManager>();
	auto vShadowMaterialGroup = pShadowMapManager->GetAllShadowGroup();
	m_pShadowCulling->SetViewport(pViewport);
	m_pShadowCulling->SetCamera(pCameraEntity);
	m_pShadowCulling->SetDirectionalLight(pMainDirectionalLight);
	m_pShadowCulling->Culling(vShadowMaterialGroup);

	m_renderInfo.shadowRenderInfo = m_pShadowCulling->GetCascadedRenderData();

	//Scene Culling.
	auto* pMeshInstanceMeshManager = pScene->GetManager<MMeshInstanceManager>();
	const std::vector<MMaterialBatchGroup*> vMaterialGroup = pMeshInstanceMeshManager->GetAllMaterialGroup();

    const MCameraFrustum cameraFrustum = MRenderSystem::GetCameraFrustum(pViewport
		, pCameraEntity->GetComponent<MCameraComponent>()
		, pCameraSceneComponent
	);

	m_pCameraFrustumCulling->SetCommand(pPrimaryCommand);
	m_pCameraFrustumCulling->SetCameraFrustum(cameraFrustum);
	m_pCameraFrustumCulling->SetCameraPosition(pCameraSceneComponent->GetWorldPosition());
	m_pCameraFrustumCulling->Culling(vMaterialGroup);

	//Voxelizer Setting.
	auto pVoxelTableBuffer = GetRenderWork<MVoxelizerRenderWork>()->GetVoxelTableBuffer();
	MORTY_ASSERT(pVoxelTableBuffer);

	//TODO: remove test code.
	const float fVoxelTableSize = MRenderGlobal::VOXEL_TABLE_SIZE;

	const MVoxelMapSetting voxelSetting = {
		Vector3(0, 0, 0), fVoxelTableSize,
		1.0f, pVoxelTableBuffer
	};

	m_renderInfo.voxelSetting = voxelSetting;

	//Voxelizer Culling.
	m_pVoxelizerCulling->SetBounds(MBoundsAABB(voxelSetting.f3VoxelOrigin, voxelSetting.f3VoxelOrigin + fVoxelTableSize));
	m_pVoxelizerCulling->Culling(vMaterialGroup);

	//Update Shader Params.
	UpdateFrameParams(m_renderInfo);

	//Resize FrameBuffer.
	const Vector2 v2ViewportSize = pViewport->GetSize();
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

	InitializeRenderWork();
	InitializeFrameShaderParams();
	InitializeRenderTarget();
}

void MDeferredRenderProgram::OnDelete()
{
	Super::OnDelete();

	ReleaseRenderWork();
	ReleaseRenderTarget();
	ReleaseFrameShaderParams();
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
	RegisterRenderWork<MVoxelizerRenderWork>();

}

void MDeferredRenderProgram::ReleaseRenderWork()
{
	for (const auto& pr : m_tRenderWork)
	{
		pr.second->Release(GetEngine());
	}
	m_tRenderWork.clear();
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

	GetRenderWork<MVoxelizerRenderWork>()->SetRenderTarget({}, {});
	
	GetRenderWork<MGBufferRenderWork>()->SetRenderTarget(vBackTextures, { pDepthTexture, {true, false, MColor::Black_T} });
	GetRenderWork<MDeferredLightingRenderWork>()->SetRenderTarget({{pLightningRenderTarget, {true, false, MColor::Black_T }} });
	GetRenderWork<MForwardRenderWork>()->SetRenderTarget(
		{ {pLightningRenderTarget, {false, true, MColor::Black_T }} },
		{ pDepthTexture, {false, true, MColor::Black_T} });

	GetRenderWork<MVoxelizerRenderWork>()->SetRenderTarget(
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
	m_pFramePropertyAdapter = std::make_shared<MFrameShaderPropertyBlock>();
	m_pFramePropertyAdapter->Initialize(GetEngine());

	m_pShadowPropertyAdapter = std::make_shared<MShadowMapShaderPropertyBlock>();
	m_pShadowPropertyAdapter->Initialize(GetEngine());

	m_pShadowCulling = std::make_shared<MCascadedShadowCulling>();
	m_pShadowCulling->Initialize(GetEngine());

	m_pVoxelizerCulling = std::make_shared<MBoundingBoxCulling>();
	m_pVoxelizerCulling->Initialize(GetEngine());

#if GPU_CULLING_ENABLE
	m_pCameraFrustumCulling = std::make_shared<MGPUCameraFrustumCulling>();
	m_pCameraFrustumCulling->Initialize(GetEngine());
#else
	m_pCameraFrustumCulling = std::make_shared<MCPUCameraFrustumCulling>();
	m_pCameraFrustumCulling->Initialize(GetEngine());
#endif

}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
	m_pFramePropertyAdapter->Release(GetEngine());
	m_pFramePropertyAdapter = nullptr;

	m_pShadowPropertyAdapter->Release(GetEngine());
	m_pShadowPropertyAdapter = nullptr;

	m_pShadowCulling->Release();
	m_pShadowCulling = nullptr;

	m_pVoxelizerCulling->Release();
	m_pVoxelizerCulling = nullptr;

	m_pCameraFrustumCulling->Release();
	m_pCameraFrustumCulling = nullptr;
}

void MDeferredRenderProgram::UpdateFrameParams(MRenderInfo& info)
{
	m_pFramePropertyAdapter->UpdateShaderSharedParams(info);
	m_pShadowPropertyAdapter->UpdateShaderSharedParams(info);
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
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling);

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
	MORTY_ASSERT(GetRenderWork<MVoxelizerRenderWork>());
	
	auto pVoxelizerWork = GetRenderWork<MVoxelizerRenderWork>();

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MCullingResultSpecificMaterialRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
	});
	indirectMesh.SetInstanceCulling(m_pVoxelizerCulling);
	indirectMesh.SetMaterial(pVoxelizerWork->GetVoxelizerMaterial());

	pVoxelizerWork->Render(m_renderInfo, {
		&indirectMesh,
	});


	MIndirectIndexRenderable debugRender;
	debugRender.SetMaterial(pVoxelizerWork->GetVoxelDebugMaterial());
	debugRender.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter
	});
	debugRender.SetIndirectIndexBuffer(pVoxelizerWork->GetVoxelDebugBuffer());
	debugRender.SetMeshBuffer(pMeshManager->GetMeshBuffer());

	pVoxelizerWork->RenderDebugVoxel(m_renderInfo, {
	   &debugRender,
	});
}

void MDeferredRenderProgram::RenderShadow()
{
	MORTY_ASSERT(GetRenderWork<MShadowMapRenderWork>());
	
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pShadowPropertyAdapter,
	});
	indirectMesh.SetInstanceCulling(m_pShadowCulling);

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

	//Current viewport.
	MViewport* pViewport = m_renderInfo.pViewport;
	MScene* pScene = pViewport->GetScene();
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		m_pFramePropertyAdapter,
	});
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDefault));
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling);


	MSkyBoxRenderable skyBox;
	skyBox.SetScene(pScene);
	skyBox.SetFramePropertyBlockAdapter(m_pFramePropertyAdapter);


	GetRenderWork<MForwardRenderWork>()->Render(m_renderInfo, {
		&indirectMesh,
		&skyBox,
		});
}

void MDeferredRenderProgram::RenderTransparent()
{
	if (GetRenderWork<MTransparentRenderWork>())
	{
		GetRenderWork<MTransparentRenderWork>()->Render(m_renderInfo);
	}
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
	indirectMesh.SetInstanceCulling(m_pCameraFrustumCulling);

	GetRenderWork<MDebugRenderWork>()->Render(m_renderInfo, {
		&indirectMesh
		});
}
