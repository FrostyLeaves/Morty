#include "MDeferredRenderProgram.h"

#include "Culling/MBoundingBoxCulling.h"
#include "Render/MRenderGlobal.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/RenderGraph/MRenderGraphWalker.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
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
#include <memory>

#include "RenderGraph/MRenderGraph.h"
#include "RenderGraph/MRenderOutputBindingWalker.h"
#include "RenderGraph/MRenderTargetManager.h"
#include "RenderWork/MDeepPeelRenderWork.h"
#include "RenderWork/MEdgeDetectionRenderWork.h"
#include "RenderWork/MToneMappingRenderWork.h"

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)


void MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
	if (!GetViewport())
		return;

	RenderSetup(pPrimaryCommand);

	m_pRenderGraph->SetFrameProperty(m_pFramePropertyAdapter);
	m_pRenderGraph->SetCameraCullingResult(m_pCameraFrustumCulling->Get());
	m_pRenderGraph->SetShadowCullingResult(m_pShadowCulling->Get());

#if MORTY_VXGI_ENABLE
	m_pRenderGraph->SetVoxelizerCullingResult(m_pVoxelizerCulling->Get());
#endif

	MRenderGraphWalker walker(m_renderInfo);
	walker(m_pRenderGraph.get());
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

	//Culling.
	MMultiThreadTaskGraphWalker walker(GetEngine()->GetThreadPool());
	walker(m_pCullingTask.get());
	m_renderInfo.shadowRenderInfo = m_pShadowCulling->Get()->GetCascadedRenderInfo();

	//Update Shader Params.
	m_pFramePropertyAdapter->UpdateShaderSharedParams(m_renderInfo);

	//Resize FrameBuffer.
	const Vector2i v2ViewportSize = pViewport->GetSize();
	m_pRenderGraph->Resize(v2ViewportSize);
}

std::shared_ptr<MTexture> MDeferredRenderProgram::GetOutputTexture()
{
	return m_pRenderGraph->GetRenderTargetManager()->FindRenderTexture(MToneMappingRenderWork::ToneMappingResult);
}

std::vector<std::shared_ptr<MTexture>> MDeferredRenderProgram::GetOutputTextures()
{
	return m_pRenderGraph->GetRenderTargetManager()->GetOutputTextures();
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


#if MORTY_VXGI_ENABLE
	RegisterRenderWork<MVoxelizerRenderWork>();
#endif

}

void MDeferredRenderProgram::ReleaseRenderWork()
{
}

void MDeferredRenderProgram::InitializeTaskGraph()
{
	m_pCullingTask = std::make_unique<MTaskGraph>();
	m_pRenderGraph = std::make_unique<MRenderGraph>(GetEngine());

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
	m_pRenderGraph = nullptr;
}

void MDeferredRenderProgram::InitializeRenderTarget()
{
    
	auto pShadowMapNode = RegisterRenderWork<MShadowMapRenderWork>();
	pShadowMapNode->GetRenderOutput(0)->SetRenderTarget(
		m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MShadowMapRenderWork::ShadowMapBufferOutput)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
		->InitTextureDesc(
			MTexture::CreateShadowMapArray(MRenderGlobal::SHADOW_TEXTURE_SIZE, MRenderGlobal::CASCADED_SHADOW_MAP_NUM)
			.InitName("Cascaded Shadow Map")
		)
	);

	auto pGBufferNode = RegisterRenderWork<MGBufferRenderWork>();

	const std::vector<MStringId> vTextureDesc = {
		MGBufferRenderWork::GBufferAlbedoMetallic,
	    MGBufferRenderWork::GBufferNormalRoughness,
	    MGBufferRenderWork::GBufferPositionAmbientOcc,
	};

	for (size_t nIdx = 0; nIdx < vTextureDesc.size(); ++nIdx)
	{
		const auto& desc = vTextureDesc[nIdx];

		pGBufferNode->GetRenderOutput(nIdx)->SetRenderTarget(
			m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(desc)
			->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
			->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
			->InitTextureDesc(MTexture::CreateRenderTargetGBuffer().InitName(desc.ToString()))
		);
	}

	auto pFinalDepthBuffer = m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MStringId("Depth Buffer"))
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateDepthBuffer().InitName("Final Depth Texture"));

	pGBufferNode->GetRenderOutput(vTextureDesc.size())->SetRenderTarget(pFinalDepthBuffer);


	auto pLightingOutputTarget = m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MDeferredLightingRenderWork::DeferredLightingOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_FLOAT_16).InitName("Deferred Lighting Output"));

	auto pDeferredLightingNode = RegisterRenderWork<MDeferredLightingRenderWork>();
	pDeferredLightingNode->GetRenderOutput(0)->SetRenderTarget(
		pLightingOutputTarget
	);

	auto pForwardRenderNode = RegisterRenderWork<MForwardRenderWork>();
	pForwardRenderNode->GetRenderOutput(0)->SetRenderTarget(
		pLightingOutputTarget
	);

	pForwardRenderNode->GetRenderOutput(1)->SetRenderTarget(
		pFinalDepthBuffer
	);

	auto pDeepPeelNode = RegisterRenderWork<MDeepPeelRenderWork>();

	pDeepPeelNode->GetRenderOutput(0)->SetRenderTarget(
		m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MDeepPeelRenderWork::FrontTextureOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Deep Peel Front Output"))
	);

	pDeepPeelNode->GetRenderOutput(1)->SetRenderTarget(
		m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MDeepPeelRenderWork::FrontTextureOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Deep Peel Back Output"))
	);

	for (size_t nIdx = 0; nIdx < 4; ++nIdx)
	{
		pDeepPeelNode->GetRenderOutput(nIdx + 2)->SetRenderTarget(
			m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MDeepPeelRenderWork::DepthOutput[nIdx])
			->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
			->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
			->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ER_FLOAT_32).InitName(fmt::format("Deep Peel Depth {}", nIdx)))
		);
	}

	auto pTransparentNode = RegisterRenderWork<MTransparentRenderWork>();
	pTransparentNode->GetRenderOutput(0)->SetRenderTarget(
		pLightingOutputTarget
	);


#if MORTY_VXGI_ENABLE

	auto pVoxelizerNode = RegisterRenderWork<MVoxelizerRenderWork>();
	pVoxelizerNode->GetRenderOutput(0)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MVoxelizerRenderWork::VoxelizerBufferOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8)
			.InitName("Voxelizer Back Texture")
			.InitSize(Vector2i(MRenderGlobal::VOXEL_VIEWPORT_SIZE, MRenderGlobal::VOXEL_VIEWPORT_SIZE)))
	);

	auto pVoxelDebugTaskNode = RegisterRenderWork<MVoxelDebugRenderWork>();
	pVoxelDebugTaskNode->GetRenderOutput(0)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MVoxelDebugRenderWork::BackBufferOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Voxel Debug Back Texture")));

	pVoxelDebugTaskNode->GetRenderOutput(1)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MVoxelDebugRenderWork::DepthBufferOutput)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
		->InitTextureDesc(MTexture::CreateShadowMapArray(1, 1).InitName("Voxel Debug Depth Texture")));

#endif

	auto pFinalBackBuffer = m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MStringId("Final Depth Buffer"))
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Post Process Output"));

	auto pEdgeDetectionNode = RegisterRenderWork<MEdgeDetectionRenderWork>();
	pEdgeDetectionNode->GetRenderOutput(0)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MEdgeDetectionRenderWork::EdgeDetectionResult)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Edge Detection Buffer")));

	auto pToneMappingNode = RegisterRenderWork<MToneMappingRenderWork>();
	pToneMappingNode->GetRenderOutput(0)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MToneMappingRenderWork::ToneMappingResult)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
		->InitTextureDesc(MTexture::CreateRenderTarget(METextureLayout::ERGBA_UNORM_8).InitName("Tone Mapping Buffer")));

	auto pDebugNode = RegisterRenderWork<MDebugRenderWork>();
	pDebugNode->GetRenderOutput(0)->SetRenderTarget(pFinalBackBuffer);
	pDebugNode->GetRenderOutput(1)->SetRenderTarget(pFinalDepthBuffer);



#if VRS_OPTIMIZE_ENABLE
	auto pVRSTextureNode = RegisterRenderWork<MVRSTextureRenderWork>();

	Vector2i n2TexelSize = GetEngine()->FindSystem<MRenderSystem>()->GetDevice()->GetShadingRateTextureTexelSize();
	pVRSTextureNode->GetRenderOutput(0)->SetRenderTarget(m_pRenderGraph->GetRenderTargetManager()->CreateRenderTarget(MVRSTextureRenderWork::VRS_TEXTURE)
		->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
		->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f / n2TexelSize.x, n2TexelSize.x)
		->InitTextureDesc(MTexture::CreateShadingRate().InitName("VRS Buffer")));
#endif

	//Bind Render Target
	MRenderOutputBindingWalker()(m_pRenderGraph.get());
	m_pRenderTargetBinding = std::make_unique<MRenderTargetBindingWalker>(GetEngine());
	(*m_pRenderTargetBinding)(m_pRenderGraph.get());
}

void MDeferredRenderProgram::ReleaseRenderTarget()
{
	m_pRenderGraph = nullptr;

	m_pRenderTargetBinding = nullptr;
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
