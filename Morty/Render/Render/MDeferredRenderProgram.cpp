#include "MDeferredRenderProgram.h"
#include "Utility/MGlobal.h"
#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/MMeshInstanceManager.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MSceneComponent.h"
#include "Culling/MCPUCameraFrustumCulling.h"
#include "Culling/MGPUCameraFrustumCulling.h"
#include "Engine/MEngine.h"
#include "Material/MComputeDispatcher.h"
#include "MeshRender/MCullingResultRenderable.h"
#include "RHI/MRenderCommand.h"
#include "Render/MFrameShaderPropertyBlock.h"
#include "Render/RenderGraph/MRenderGraphWalker.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "RenderGraph/MRenderGraph.h"
#include "RenderGraph/MRenderOutputBindingWalker.h"
#include "RenderNode/MDebugRenderNode.h"
#include "RenderNode/MDeepPeelRenderNode.h"
#include "RenderNode/MDeferredLightingRenderNode.h"
#include "RenderNode/MEdgeDetectionRenderNode.h"
#include "RenderNode/MForwardRenderNode.h"
#include "RenderNode/MGBufferRenderNode.h"
#include "RenderNode/MHBAOBlurRenderNode.h"
#include "RenderNode/MHBAORenderNode.h"
#include "RenderNode/MShadowMapRenderNode.h"
#include "RenderNode/MToneMappingRenderNode.h"
#include "RenderNode/MTransparentRenderNode.h"
#include "Scene/MScene.h"
#include "Shadow/MShadowMeshManager.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MMultiThreadTaskGraphWalker.h"
#include "TaskGraph/MTaskNode.h"
#include "Utility/MBounds.h"
#include "Flatbuffer/MRenderGraph_generated.h"
#include "Flatbuffer/MTaskNode_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MDeferredRenderProgram, MIRenderProgram)

const MStringId FinalBackBuffer = MStringId("Final Back Buffer");

void            MDeferredRenderProgram::Render(MIRenderCommand* pPrimaryCommand)
{
    if (!GetViewport()) return;
    if (m_renderGraph->NeedCompile()) { m_renderGraph->Compile(); }
    if (m_renderGraph->GetFinalNodes().empty()) { return; }

    RenderSetup(pPrimaryCommand);

    m_renderGraph->SetFrameProperty(m_framePropertyAdapter);
    m_renderGraph->SetCameraCullingResult(m_cameraFrustumCulling->Get());
    m_renderGraph->SetShadowCullingResult(m_shadowCulling->Get());

#if MORTY_VXGI_ENABLE
    m_renderGraph->SetVoxelizerCullingResult(m_voxelizerCulling->Get());
#endif

    //Run Render Graph
    MRenderGraphWalker walker(m_renderInfo);
    walker(m_renderGraph.get());
}

void MDeferredRenderProgram::RenderSetup(MIRenderCommand* pPrimaryCommand)
{

    MViewport* pViewport             = GetViewport();
    MEntity*   pCameraEntity         = pViewport->GetCamera();
    MScene*    pScene                = pViewport->GetScene();
    MEntity*   pMainDirectionalLight = pScene->FindFirstEntityByComponent<MDirectionalLightComponent>();
    auto*      pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();

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
    GetRenderNode<MVoxelizerRenderNode>()->SetupVoxelSetting(
            m_renderInfo.m4CameraTransform.GetTranslation(),
            nClipmapIdx
    );
    auto voxelizerBounds = GetRenderNode<MVoxelizerRenderNode>()->GetVoxelizerBoundsAABB(nClipmapIdx);
    m_voxelizerCulling->SetInput(vMaterialGroup);
    //Voxelizer Culling.
    m_voxelizerCulling->Get()->SetBounds(voxelizerBounds);
#endif

    //Culling Task.
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

void MDeferredRenderProgram::OnCreated()
{
    Super::OnCreated();

    InitializeTaskGraph();
    InitializeFrameShaderParams();
    InitializeRenderGraph();
}

void MDeferredRenderProgram::OnDelete()
{
    Super::OnDelete();

    ReleaseTaskGraph();
    ReleaseFrameShaderParams();
    ReleaseRenderGraph();
}

void MDeferredRenderProgram::InitializeTaskGraph()
{
    m_cullingTask = std::make_unique<MTaskGraph>();

    m_shadowCulling = m_cullingTask->AddNode<MCullingTaskNode<MCascadedShadowCulling>>(MStringId("Shadow Culling"));
    m_shadowCulling->Initialize(GetEngine());

#if MORTY_VXGI_ENABLE
    m_voxelizerCulling = m_cullingTask->AddNode<MCullingTaskNode<MBoundingBoxCulling>>(MStringId("Voxelizer Culling"));
    m_voxelizerCulling->Initialize(GetEngine());
#endif

    m_cameraFrustumCulling =
            m_cullingTask->AddNode<MCullingTaskNode<CameraFrustumCullingType>>(MStringId("Camera Culling"));
    m_cameraFrustumCulling->Initialize(GetEngine());
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
    m_renderGraph = std::make_unique<MRenderGraph>(GetEngine());


    /*
    auto rtManager = m_renderGraph->GetRenderTargetManager();

    // final back buffer.
    auto pFinalBackBuffer = rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
            .name         = FinalBackBuffer,
            .scale        = 1.0f,
            .resizePolicy = MEResizePolicy::Scale,
            .sharedPolicy = MESharedPolicy::Shared,
            .textureDesc  = MTexture::CreateRenderTarget("Final Output", METextureFormat::UNorm_RGBA8),
    });

    // final depth buffer.
    auto pFinalDepthBuffer = rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
            .name         = MStringId("Depth Buffer"),
            .scale        = 1.0f,
            .resizePolicy = MEResizePolicy::Scale,
            .sharedPolicy = MESharedPolicy::Shared,
            .textureDesc  = MTexture::CreateDepthBuffer("Final Depth Texture"),
    });
    //lighting output buffer.
    auto pLightingOutputTarget = rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
            .name         = MDeferredLightingRenderNode::DeferredLightingOutput,
            .scale        = 1.0f,
            .resizePolicy = MEResizePolicy::Scale,
            .sharedPolicy = MESharedPolicy::Shared,
            .textureDesc  = MTexture::CreateRenderTarget("Deferred Lighting Output", METextureFormat::Float_RGBA16),
    });

    auto pShadowMapNode = RegisterRenderNode<MShadowMapRenderNode>();
    pShadowMapNode->GetRenderOutput(0)->SetRenderTarget(
            rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                    .name         = MShadowMapRenderNode::ShadowMapBufferOutput,
                    .scale        = 1.0f,
                    .resizePolicy = MEResizePolicy::Fixed,
                    .sharedPolicy = MESharedPolicy::Exclusive,
                    .textureDesc  = MTexture::CreateShadowMapArray(
                            "Cascaded Shadow Map",
                            MRenderGlobal::SHADOW_TEXTURE_SIZE,
                            MRenderGlobal::CASCADED_SHADOW_MAP_NUM
                    ),
            })
    );

    const std::vector<MStringId> vTextureDesc = {
            MGBufferRenderNode::GBufferAlbedoMetallic,
            MGBufferRenderNode::GBufferNormalRoughness,
            MGBufferRenderNode::GBufferPositionAmbientOcc,
    };

    auto pGBufferNode = RegisterRenderNode<MGBufferRenderNode>();
    for (size_t nIdx = 0; nIdx < vTextureDesc.size(); ++nIdx)
    {
        const auto& rtName = vTextureDesc[nIdx];
        pGBufferNode->GetRenderOutput(nIdx)->SetRenderTarget(
                rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                        .name         = rtName,
                        .scale        = 1.0f,
                        .resizePolicy = MEResizePolicy::Scale,
                        .sharedPolicy = MESharedPolicy::Shared,
                        .textureDesc  = MTexture::CreateRenderTargetGBuffer(rtName.ToString()),
                })
        );
    }
    pGBufferNode->GetRenderOutput(vTextureDesc.size())->SetRenderTarget(pFinalDepthBuffer);


    auto pDeferredLightingNode = RegisterRenderNode<MDeferredLightingRenderNode>();
    pDeferredLightingNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);

    auto pForwardRenderNode = RegisterRenderNode<MForwardRenderNode>();
    pForwardRenderNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);
    pForwardRenderNode->GetRenderOutput(1)->SetRenderTarget(pFinalDepthBuffer);

    auto pDeepPeelNode = RegisterRenderNode<MDeepPeelRenderNode>();

    pDeepPeelNode->GetRenderOutput(0)->SetRenderTarget(
            rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                    .name         = MDeepPeelRenderNode::FrontTextureOutput,
                    .scale        = 1.0f,
                    .resizePolicy = MEResizePolicy::Scale,
                    .sharedPolicy = MESharedPolicy::Shared,
                    .textureDesc = MTexture::CreateRenderTarget("Deep Peel Front Output", METextureFormat::UNorm_RGBA8),
            })
    );

    pDeepPeelNode->GetRenderOutput(1)->SetRenderTarget(
            rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                    .name         = MDeepPeelRenderNode::BackTextureOutput,
                    .scale        = 1.0f,
                    .resizePolicy = MEResizePolicy::Scale,
                    .sharedPolicy = MESharedPolicy::Shared,
                    .textureDesc  = MTexture::CreateRenderTarget("Deep Peel Back Output", METextureFormat::UNorm_RGBA8),
            })
    );

    for (size_t nIdx = 0; nIdx < 4; ++nIdx)
    {
        pDeepPeelNode->GetRenderOutput(nIdx + 2)->SetRenderTarget(
                rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                        .name         = MDeepPeelRenderNode::DepthOutput[nIdx],
                        .scale        = 1.0f,
                        .resizePolicy = MEResizePolicy::Scale,
                        .sharedPolicy = MESharedPolicy::Shared,
                        .textureDesc  = MTexture::CreateRenderTarget(
                                fmt::format("Deep Peel Depth {}", nIdx),
                                METextureFormat::Float_R32
                        ),
                })
        );
    }

    auto pTransparentNode = RegisterRenderNode<MTransparentRenderNode>();
    pTransparentNode->GetRenderOutput(0)->SetRenderTarget(pLightingOutputTarget);

#if MORTY_VXGI_ENABLE

    auto pVoxelizerNode = RegisterRenderNode<MVoxelizerRenderNode>();
    pVoxelizerNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelizerRenderNode::VoxelizerBufferOutput)
                    ->InitSharedPolicy(MESharedPolicy::Exclusive)
                    ->InitResizePolicy(MEResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8)
                                              .InitName("Voxelizer Back Texture")
                                              .InitSize(Vector2i(
                                                      MRenderGlobal::VOXEL_VIEWPORT_SIZE,
                                                      MRenderGlobal::VOXEL_VIEWPORT_SIZE
                                              )))
    );

    auto pVoxelDebugTaskNode = RegisterRenderNode<MVoxelDebugRenderNode>();
    pVoxelDebugTaskNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelDebugRenderNode::BackBufferOutput)
                    ->InitSharedPolicy(MESharedPolicy::Exclusive)
                    ->InitResizePolicy(MEResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateRenderTarget(METextureFormat::UNorm_RGBA8)
                                              .InitName("Voxel Debug Back Texture"))
    );

    pVoxelDebugTaskNode->GetRenderOutput(1)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MVoxelDebugRenderNode::DepthBufferOutput)
                    ->InitSharedPolicy(MESharedPolicy::Exclusive)
                    ->InitResizePolicy(MEResizePolicy::Fixed)
                    ->InitTextureDesc(MTexture::CreateShadowMapArray(1, 1).InitName("Voxel Debug Depth Texture"))
    );

#endif

    auto pEdgeDetectionNode = RegisterRenderNode<MEdgeDetectionRenderNode>();
    pEdgeDetectionNode->GetRenderOutput(0)->SetRenderTarget(
            rtManager->CreateRenderTarget(MRenderTargetManager::RenderTargetDesc{
                    .name         = MEdgeDetectionRenderNode::EdgeDetectionResult,
                    .scale        = 1.0f,
                    .resizePolicy = MEResizePolicy::Scale,
                    .sharedPolicy = MESharedPolicy::Shared,
                    .textureDesc  = MTexture::CreateRenderTarget("Edge Detection Buffer", METextureFormat::UNorm_RGBA8),
            })
    );

    auto pToneMappingNode = RegisterRenderNode<MToneMappingRenderNode>();
    pToneMappingNode->GetRenderOutput(0)->SetRenderTarget(pFinalBackBuffer);

    auto pDebugNode = RegisterRenderNode<MDebugRenderNode>();
    pDebugNode->GetRenderOutput(0)->SetRenderTarget(pFinalBackBuffer);
    pDebugNode->GetRenderOutput(1)->SetRenderTarget(pFinalDepthBuffer);


#if VRS_OPTIMIZE_ENABLE
    auto     pVRSTextureNode = RegisterRenderNode<MVRSTextureRenderNode>();

    Vector2i n2TexelSize = GetEngine()->FindSystem<MRenderSystem>()->GetDevice()->GetShadingRateTextureTexelSize();
    pVRSTextureNode->GetRenderOutput(0)->SetRenderTarget(
            m_renderGraph->GetRenderTargetManager()
                    ->CreateRenderTarget(MRenderGraphName::VRS_TEXTURE)
                    ->InitSharedPolicy(MESharedPolicy::Shared)
                    ->InitResizePolicy(MEResizePolicy::Scale, 1.0f / n2TexelSize.x, n2TexelSize.x)
                    ->InitTextureDesc(MTexture::CreateShadingRate().InitName("VRS Buffer"))
    );
#endif

    //Bind Render Target
    MRenderOutputBindingWalker()(m_renderGraph.get());
    (*m_renderTargetBinding)(m_renderGraph.get());

     */
}

void MDeferredRenderProgram::ReleaseRenderGraph() { m_renderGraph = nullptr; }

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
void MDeferredRenderProgram::LoadGraph(const std::vector<MByte>& buffer)
{
    ReleaseRenderGraph();
    ReleaseFrameShaderParams();

    InitializeFrameShaderParams();
    InitializeRenderGraph();


    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushBytes((const uint8_t*) buffer.data(), buffer.size());
    const fbs::MRenderGraph* fbRenderGraph = fbs::GetMRenderGraph(fbb.GetCurrentBufferPointer());

    m_renderGraph->Deserialize(fbRenderGraph);
}

void MDeferredRenderProgram::SaveGraph(std::vector<MByte>& output)
{
    output.clear();

    flatbuffers::FlatBufferBuilder fbb;
    auto                           allTaskNode = m_renderGraph->GetAllNodes();

    fbb.Finish(m_renderGraph->Serialize(fbb));

    output.resize(fbb.GetSize());
    memcpy(output.data(), fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));
}
