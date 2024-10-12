#include "MDeferredRenderProgram.h"

#include "Utility/MRenderGlobal.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Culling/MBoundingBoxCulling.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RenderProgram/MFrameShaderPropertyBlock.h"
#include "RenderProgram/RenderGraph/MRenderGraphWalker.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "Scene/MScene.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MBounds.h"
#include "Utility/MFunction.h"

#include "Basic/MCameraFrustum.h"
#include "RHI/MRenderCommand.h"

#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MSkyBoxComponent.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "RenderWork/MDebugRenderWork.h"
#include "RenderWork/MDeferredLightingRenderWork.h"
#include "RenderWork/MForwardRenderWork.h"
#include "RenderWork/MGBufferRenderWork.h"
#include "RenderWork/MShadowMapRenderWork.h"
#include "RenderWork/MTransparentRenderWork.h"
#include "RenderWork/MVoxelDebugRenderWork.h"
#include "RenderWork/MVoxelizerRenderWork.h"
#include "Shadow/MShadowMeshManager.h"

#include "Batch/MMeshInstanceManager.h"
#include "MeshRender/MCullingResultRenderable.h"
#include "MeshRender/MCullingResultSpecificMaterialRenderable.h"
#include "MeshRender/MIndirectIndexRenderable.h"
#include "MeshRender/MSkyBoxRenderable.h"

#include "Culling/MCPUCameraFrustumCulling.h"
#include "Culling/MGPUCameraFrustumCulling.h"
#include "Manager/MAnimationManager.h"

#include "Utility/MGlobal.h"
#include "Culling/MBoundingBoxCulling.h"
#include "Manager/MEnvironmentManager.h"
#include "RenderWork/MVRSTextureRenderWork.h"
#include "TaskGraph/MMultiThreadTaskGraphWalker.h"
#include <memory>

#include "RenderGraph/MRenderGraph.h"
#include "RenderGraph/MRenderOutputBindingWalker.h"
#include "RenderGraph/MRenderTargetManager.h"
#include "RenderWork/MDeepPeelRenderWork.h"
#include "RenderWork/MEdgeDetectionRenderWork.h"
#include "RenderWork/MHBAOBlurRenderWork.h"
#include "RenderWork/MHBAORenderWork.h"
#include "RenderWork/MToneMappingRenderWork.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

const MStringId FinalBackBuffer = MStringId("Final Back Buffer");

void            MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
    if (!GetViewport()) return;

    RenderSetup(pPrimaryCommand);

    m_renderGraph->SetFrameProperty(m_framePropertyAdapter);
    m_renderGraph->SetCameraCullingResult(m_cameraFrustumCulling->Get());
    m_renderGraph->SetShadowCullingResult(m_shadowCulling->Get());

#if MORTY_VXGI_ENABLE
    m_renderGraph->SetVoxelizerCullingResult(m_voxelizerCulling->Get());
#endif

    MRenderGraphWalker walker(m_renderInfo);
    walker(m_renderGraph.get());
}

void MDeferredRenderProgram::RenderSetup(MIRenderCommand* pPrimaryCommand)
{
    MViewport*       pViewport             = GetViewport();
    MEntity*         pCameraEntity         = pViewport->GetCamera();
    MScene*          pScene                = pViewport->GetScene();
    MEntity*         pMainDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();

    m_renderInfo                       = MRenderInfo::CreateFromViewport(pViewport);
    m_renderInfo.nFrameIndex           = m_frameIndex++;
    m_renderInfo.pPrimaryRenderCommand = pPrimaryCommand;


    //Shadow map Culling.
    auto* pShadowMapManager    = pViewport->GetScene()->GetManager<MShadowMeshManager>();
    auto  vShadowMaterialGroup = pShadowMapManager->GetAllShadowGroup();
    m_shadowCulling->Get()->SetCamera(pCameraEntity);
    m_shadowCulling->Get()->SetViewport(pViewport);
    m_shadowCulling->Get()->SetDirectionalLight(pMainDirectionalLight);
    m_shadowCulling->SetInput(vShadowMaterialGroup);


    //Scene Culling.
    auto*                                   pMeshInstanceMeshManager = pScene->GetManager<MMeshInstanceManager>();
    const std::vector<MMaterialBatchGroup*> vMaterialGroup           = pMeshInstanceMeshManager->GetAllMaterialGroup();

    const MCameraFrustum                    cameraFrustum = MRenderSystem::GetCameraFrustum(
            pViewport,
            pCameraEntity->GetComponent<MCameraComponent>(),
            pCameraSceneComponent
    );

    m_cameraFrustumCulling->Get()->SetCommand(pPrimaryCommand);
    m_cameraFrustumCulling->Get()->SetCameraFrustum(cameraFrustum);
    m_cameraFrustumCulling->Get()->SetCameraPosition(pCameraSceneComponent->GetWorldPosition());
    m_cameraFrustumCulling->SetInput(vMaterialGroup);

#if MORTY_VXGI_ENABLE
    uint32_t nClipmapIdx = m_renderInfo.nFrameIndex % MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM;
    GetRenderWork<MVoxelizerRenderWork>()->SetupVoxelSetting(
            m_renderInfo.m4CameraTransform.GetTranslation(),
            nClipmapIdx
    );
    auto voxelizerBounds = GetRenderWork<MVoxelizerRenderWork>()->GetVoxelizerBoundsAABB(nClipmapIdx);
    m_voxelizerCulling->SetInput(vMaterialGroup);
    //Voxelizer Culling.
    m_voxelizerCulling->Get()->SetBounds(voxelizerBounds);
#endif

    //Culling.
    MMultiThreadTaskGraphWalker walker(GetEngine()->GetThreadPool());
    walker(m_cullingTask.get());
    m_renderInfo.shadowRenderInfo = m_shadowCulling->Get()->GetCascadedRenderInfo();

    //TODO: uploadBuffer use render command.
    m_shadowCulling->Get()->UploadBuffer(pPrimaryCommand);
    m_cameraFrustumCulling->Get()->UploadBuffer(pPrimaryCommand);

#if MORTY_VXGI_ENABLE
    m_voxelizerCulling->Get()->UploadBuffer(pPrimaryCommand);
#endif

    //Update Shader Params.
    m_framePropertyAdapter->UpdateShaderSharedParams(m_renderInfo);

    //Resize FrameBuffer.
    const Vector2i v2ViewportSize = pViewport->GetSize();
    m_renderGraph->Resize(v2ViewportSize);


    MRenderGraphSetupWalker setupWalker(m_renderInfo);
    setupWalker(m_renderGraph.get());
}

std::shared_ptr<MTexture> MDeferredRenderProgram::GetOutputTexture()
{
    return m_renderGraph->GetRenderTargetManager()->FindRenderTexture(FinalBackBuffer);
}

std::vector<std::shared_ptr<MTexture>> MDeferredRenderProgram::GetOutputTextures()
{
    return m_renderGraph->GetRenderTargetManager()->GetOutputTextures();
}

void MDeferredRenderProgram::OnCreated()
{
    Super::OnCreated();

    InitializeTaskGraph();
    InitializeFrameShaderParams();
    InitializeRenderWork();
    InitializeRenderGraph();
}

void MDeferredRenderProgram::OnDelete()
{
    Super::OnDelete();

    ReleaseTaskGraph();
    ReleaseFrameShaderParams();
    ReleaseRenderWork();
    ReleaseRenderGraph();
}

void MDeferredRenderProgram::InitializeRenderWork()
{


#if MORTY_VXGI_ENABLE
    RegisterRenderWork<MVoxelizerRenderWork>();
#endif
}

void MDeferredRenderProgram::ReleaseRenderWork() {}

void MDeferredRenderProgram::InitializeTaskGraph()
{
    m_cullingTask = std::make_unique<MTaskGraph>();
    m_renderGraph = std::make_unique<MRenderGraph>(GetEngine());

    m_shadowCulling = m_cullingTask->AddNode<MCullingTaskNode<MCascadedShadowCulling>>(MStringId("Shadow Culling"));
    m_shadowCulling->Initialize(GetEngine());

#if MORTY_VXGI_ENABLE
    m_voxelizerCulling = m_cullingTask->AddNode<MCullingTaskNode<MBoundingBoxCulling>>(MStringId("Voxelizer Culling"));
    m_voxelizerCulling->Initialize(GetEngine());
#endif

#if GPU_CULLING_ENABLE
    m_cameraFrustumCulling =
            m_cullingTask->AddNode<MCullingTaskNode<MGPUCameraFrustumCulling>>(MStringId("Gpu Camera Culling"));
    m_cameraFrustumCulling->Initialize(GetEngine());
#else
    m_cameraFrustumCulling =
            m_cullingTask->AddNode<MCullingTaskNode<MCPUCameraFrustumCulling>>(MStringId("Cpu Camera Culling"));
    m_cameraFrustumCulling->Initialize(GetEngine());
#endif
}

void MDeferredRenderProgram::ReleaseTaskGraph()
{
    m_shadowCulling->Release();
    m_shadowCulling = nullptr;

#if MORTY_VXGI_ENABLE
    m_voxelizerCulling->Release();
    m_voxelizerCulling = nullptr;
#endif

    m_cameraFrustumCulling->Release();
    m_cameraFrustumCulling = nullptr;

    m_cullingTask = nullptr;
    m_renderGraph = nullptr;
}

void MDeferredRenderProgram::InitializeRenderGraph()
{
    auto pShadowMapNode = RegisterRenderWork<MShadowMapRenderWork>();
    pShadowMapNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MShadowMapRenderWork::ShadowMapBufferOutput)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
                    ->InitTextureDesc(MTexture::CreateShadowMapArray(
                                              MRenderGlobal::SHADOW_TEXTURE_SIZE,
                                              MRenderGlobal::CASCADED_SHADOW_MAP_NUM
                    )
                                              .InitName("Cascaded Shadow Map"))
    );

    auto                         pGBufferNode = RegisterRenderWork<MGBufferRenderWork>();

    const std::vector<MStringId> vTextureDesc = {
            MGBufferRenderWork::GBufferAlbedoMetallic,
            MGBufferRenderWork::GBufferNormalRoughness,
            MGBufferRenderWork::GBufferPositionAmbientOcc,
    };

    for (size_t nIdx = 0; nIdx < vTextureDesc.size(); ++nIdx)
    {
        const auto& desc = vTextureDesc[nIdx];

        pGBufferNode->GetRenderOutput(nIdx)->SetRenderTarget(
                m_renderGraph->GetRenderTargetManager()
                        ->CreateRenderTarget(desc)
                        ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                        ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                        ->InitTextureDesc(MTexture::CreateRenderTargetGBuffer().InitName(desc.ToString()))
        );
    }

    auto pFinalDepthBuffer = m_renderGraph->GetRenderTargetManager()
                                     ->CreateRenderTarget(MStringId("Depth Buffer"))
                                     ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                                     ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                                     ->InitTextureDesc(MTexture::CreateDepthBuffer().InitName("Final Depth Texture"));

    pGBufferNode->GetRenderOutput(vTextureDesc.size())->SetRenderTarget(pFinalDepthBuffer);

    auto pHBAONode = RegisterRenderWork<MHBAORenderWork>();
    pHBAONode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MHBAORenderWork::HBAOOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_R8).InitName("HBAO Buffer"))
    );

    auto pHbaoBlurNodeV = RegisterRenderWork<MHBAOBlurRenderWorkV>(MStringId("HBAO Blur V"));
    pHbaoBlurNodeV->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MHBAOBlurRenderWorkV::BlurOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(
                            MTexture::CreateRenderTarget(METextureFormat::UNorm_R8).InitName("HBAO Blur Buffer V")
                    )
    );
    pHbaoBlurNodeV->InitDirection(true);

    auto pHbaoBlurNodeH = RegisterRenderWork<MHBAOBlurRenderWorkH>(MStringId("HBAO Blur H"));
    pHbaoBlurNodeH->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MHBAOBlurRenderWorkH::BlurOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(
                            MTexture::CreateRenderTarget(METextureFormat::UNorm_R8).InitName("HBAO Blur Buffer H")
                    )
    );

    auto pLightingOutputTarget = m_renderGraph->GetRenderTargetManager()
                                         ->CreateRenderTarget(MDeferredLightingRenderWork::DeferredLightingOutput)
                                         ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                                         ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                                         ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::Float_RGBA16)
                                                                   .InitName("Deferred Lighting Output"));

    auto pDeferredLightingNode = RegisterRenderWork<MDeferredLightingRenderWork>();
    pDeferredLightingNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);

    auto pForwardRenderNode = RegisterRenderWork<MForwardRenderWork>();
    pForwardRenderNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);

    pForwardRenderNode->GetRenderOutput(1)->SetRenderTarget(pFinalDepthBuffer);

    auto pDeepPeelNode = RegisterRenderWork<MDeepPeelRenderWork>();

    pDeepPeelNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MDeepPeelRenderWork::FrontTextureOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8)
                                              .InitName("Deep Peel Front Output"))
    );

    pDeepPeelNode->GetRenderOutput(1)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MDeepPeelRenderWork::BackTextureOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(
                            MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8).InitName("Deep Peel Back Output")
                    )
    );

    for (size_t nIdx = 0; nIdx < 4; ++nIdx)
    {
        pDeepPeelNode->GetRenderOutput(nIdx + 2)->SetRenderTarget(
                m_renderGraph->GetRenderTargetManager()
                        ->CreateRenderTarget(MDeepPeelRenderWork::DepthOutput[nIdx])
                        ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                        ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                        ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::Float_R32)
                                                  .InitName(fmt::format("Deep Peel Depth {}", nIdx)))
        );
    }

    auto pTransparentNode = RegisterRenderWork<MTransparentRenderWork>();
    pTransparentNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);

#if MORTY_VXGI_ENABLE

    auto pVoxelizerNode = RegisterRenderWork<MVoxelizerRenderWork>();
    pVoxelizerNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelizerRenderWork::VoxelizerBufferOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8)
                                              .InitName("Voxelizer Back Texture")
                                              .InitSize(Vector2i(
                                                      MRenderGlobal::VOXEL_VIEWPORT_SIZE,
                                                      MRenderGlobal::VOXEL_VIEWPORT_SIZE
                                              )))
    );

    auto pVoxelDebugTaskNode = RegisterRenderWork<MVoxelDebugRenderWork>();
    pVoxelDebugTaskNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelDebugRenderWork::BackBufferOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8)
                                              .InitName("Voxel Debug Back Texture"))
    );

    pVoxelDebugTaskNode->GetRenderOutput(1)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelDebugRenderWork::DepthBufferOutput)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Exclusive)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateShadowMapArray(1, 1).InitName("Voxel Debug Depth Texture"))
    );

#endif

    auto pEdgeDetectionNode = RegisterRenderWork<MEdgeDetectionRenderWork>();
    pEdgeDetectionNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MEdgeDetectionRenderWork::EdgeDetectionResult)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(
                            MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8).InitName("Edge Detection Buffer")
                    )
    );

    auto pFinalBackBuffer =
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(FinalBackBuffer)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f)
                    ->InitTextureDesc(
                            MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8).InitName("Final Output")
                    );

    auto pToneMappingNode = RegisterRenderWork<MToneMappingRenderWork>();
    pToneMappingNode->GetRenderOutput(0)->SetRenderTarget(pFinalBackBuffer);

    auto pDebugNode = RegisterRenderWork<MDebugRenderWork>();
    pDebugNode->GetRenderOutput(0)->SetRenderTarget(pFinalBackBuffer);
    pDebugNode->GetRenderOutput(1)->SetRenderTarget(pFinalDepthBuffer);


#if VRS_OPTIMIZE_ENABLE
    auto     pVRSTextureNode = RegisterRenderWork<MVRSTextureRenderWork>();

    Vector2i n2TexelSize = GetEngine()->FindSystem<MRenderSystem>()->GetDevice()->GetShadingRateTextureTexelSize();
    pVRSTextureNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVRSTextureRenderWork::VRS_TEXTURE)
                    ->InitSharedPolicy(MRenderTaskTarget::SharedPolicy::Shared)
                    ->InitResizePolicy(MRenderTaskTarget::ResizePolicy::Scale, 1.0f / n2TexelSize.x, n2TexelSize.x)
                    ->InitTextureDesc(MTexture::CreateShadingRate().InitName("VRS Buffer"))
    );
#endif

    //Bind Render Target
    MRenderOutputBindingWalker()(m_renderGraph.get());
    m_renderTargetBinding = std::make_unique<MRenderTargetBindingWalker>(GetEngine());
    (*m_renderTargetBinding)(m_renderGraph.get());
}

void MDeferredRenderProgram::ReleaseRenderGraph()
{
    m_renderGraph = nullptr;

    m_renderTargetBinding = nullptr;
}

void MDeferredRenderProgram::InitializeFrameShaderParams()
{
    m_framePropertyAdapter = std::make_shared<MFrameShaderPropertyBlock>();
    m_framePropertyAdapter->Initialize(GetEngine());
    m_framePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MFramePropertyDecorator>());
    m_framePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MLightPropertyDecorator>());
    m_framePropertyAdapter->RegisterPropertyDecorator(std::make_shared<MAnimationPropertyDecorator>());
}

void MDeferredRenderProgram::ReleaseFrameShaderParams()
{
    m_framePropertyAdapter->Release(GetEngine());
    m_framePropertyAdapter = nullptr;
}
