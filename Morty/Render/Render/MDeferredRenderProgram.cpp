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

MTexturePtr MDeferredRenderProgram::GetOutputTexture()
{
    return m_renderGraph->GetRenderTargetManager()->FindRenderTexture(FinalBackBuffer);
}

MTextureArray MDeferredRenderProgram::GetOutputTextures()
{
    return m_renderGraph->GetRenderTargetManager()->GetOutputTextures();
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
    m_renderGraph         = std::make_unique<MRenderGraph>(GetEngine());
    m_renderTargetBinding = std::make_unique<MRenderTargetBindingWalker>(GetEngine());
    m_renderTargetBinding->SetForceExclusive(true);


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

void MDeferredRenderProgram::ReleaseRenderGraph()
{
    m_renderGraph         = nullptr;
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
void MDeferredRenderProgram::LoadGraph(const std::vector<MByte>& buffer)
{
    ReleaseRenderGraph();
    ReleaseFrameShaderParams();

    InitializeFrameShaderParams();
    InitializeRenderGraph();


    flatbuffers::FlatBufferBuilder fbb;
    fbb.PushBytes((const uint8_t*) buffer.data(), buffer.size());

    auto                            rtManager           = m_renderGraph->GetRenderTargetManager();
    const fbs::MRenderGraph*        fbRenderGraph       = fbs::GetMRenderGraph(fbb.GetCurrentBufferPointer());
    const auto&                     fbRenderTargetArray = *fbRenderGraph->render_target();
    const auto&                     fbRenderNodeArray   = *fbRenderGraph->node_array();

    std::vector<MRenderTaskTarget*> renderTargets(fbRenderTargetArray.size());
    std::vector<MRenderTaskNode*>   renderNodes(fbRenderNodeArray.size());

    for (size_t targetIdx = 0; targetIdx < fbRenderTargetArray.size(); ++targetIdx)
    {
        const fbs::MRenderGraphTargetDesc* fbTargetDesc =
                fbRenderTargetArray.Get(static_cast<flatbuffers::uoffset_t>(targetIdx));

        rtManager->CreateRenderTarget(*fbTargetDesc);
    }

    for (size_t nodeIdx = 0; nodeIdx < fbRenderNodeArray.size(); ++nodeIdx)
    {
        const fbs::MRenderTaskNode* fbRenderNode = fbRenderNodeArray.Get(static_cast<flatbuffers::uoffset_t>(nodeIdx));
        auto                        nodeTypeName = fbRenderNode->super()->node_type()->str();
        auto                        renderNode   = RegisterRenderNode(MStringId(nodeTypeName), nodeTypeName);
        renderNodes[nodeIdx]                     = renderNode;

        if (!fbRenderNode->output_name()) continue;

        const size_t outputNum = fbRenderNode->output_name()->size();
        for (size_t nOutputIdx = 0; nOutputIdx < outputNum; ++nOutputIdx)
        {
            if (nOutputIdx >= renderNode->GetOutputSize()) continue;

            auto outputName = fbRenderNode->output_name()->Get(nOutputIdx);
            if (!outputName) continue;
            auto renderTarget = rtManager->FindRenderTarget(MStringId(outputName->c_str()));
            if (!renderTarget) continue;

            renderNode->GetRenderOutput(nOutputIdx)->SetRenderTarget(renderTarget);
        }
    }

    for (size_t nodeIdx = 0; nodeIdx < fbRenderNodeArray.size(); ++nodeIdx)
    {
        const fbs::MRenderTaskNode* fbRenderNode = fbRenderNodeArray.Get(static_cast<flatbuffers::uoffset_t>(nodeIdx));
        const fbs::MTaskNode*       fbTaskNode   = fbRenderNode->super();
        if (fbTaskNode->link_node_id() == nullptr || fbTaskNode->link_output_id() == nullptr) continue;

        MRenderTaskNode* renderNode = renderNodes[nodeIdx];
        const size_t     connectNum = fbTaskNode->link_node_id()->size();

        for (size_t connIdx = 0; connIdx < connectNum; ++connIdx)
        {
            const auto prevNodeIdx   = fbTaskNode->link_node_id()->Get(connIdx);
            const auto prevOutputIdx = fbTaskNode->link_output_id()->Get(connIdx);
            auto       prevNode      = renderNodes[prevNodeIdx];

            prevNode->GetOutput(prevOutputIdx)->LinkTo(renderNode->GetInput(connIdx));
        }
    }

    //Bind Render Target
    (*m_renderTargetBinding)(m_renderGraph.get());
}

void MDeferredRenderProgram::SaveGraph(std::vector<MByte>& output)
{
    output.clear();

    flatbuffers::FlatBufferBuilder                                fbb;

    auto                                                          rtManager = m_renderGraph->GetRenderTargetManager();

    std::vector<flatbuffers::Offset<fbs::MRenderGraphTargetDesc>> renderTargetArray;
    for (const auto& pr: rtManager->GetRenderTargetTable())
    {
        renderTargetArray.emplace_back(MRenderTargetManager::SerializeRenderTarget(pr.second.get(), fbb).o);
    }
    auto                                                   fbRenderTargetArray = fbb.CreateVector(renderTargetArray);


    std::vector<flatbuffers::Offset<fbs::MRenderTaskNode>> renderNodeArray;
    auto                                                   allTaskNode = m_renderGraph->GetAllNodes();
    for (size_t nNodeIdx = 0; nNodeIdx < allTaskNode.size(); ++nNodeIdx)
    {
        auto                  taskNode = allTaskNode[nNodeIdx];
        std::vector<uint32_t> taskInputNode(taskNode->GetInputSize());
        std::vector<uint32_t> taskInputSlot(taskNode->GetInputSize());

        for (size_t nInputIdx = 0; nInputIdx < taskNode->GetInputSize(); ++nInputIdx)
        {
            auto prevOutput = taskNode->GetInput(nInputIdx)->GetLinkedOutput();
            auto findResult = std::find(allTaskNode.begin(), allTaskNode.end(), prevOutput->GetTaskNode());
            if (findResult != allTaskNode.end())
            {
                taskInputNode[nInputIdx] = findResult - allTaskNode.begin();
                taskInputSlot[nInputIdx] = prevOutput->GetIndex();
            }
        }

        auto                  fbTypeName  = fbb.CreateString(taskNode->GetTypeName());
        auto                  fbInputNode = fbb.CreateVector(taskInputNode);
        auto                  fbInputSlot = fbb.CreateVector(taskInputSlot);

        fbs::MTaskNodeBuilder taskNodeBuilder(fbb);
        taskNodeBuilder.add_node_id(nNodeIdx);
        taskNodeBuilder.add_node_type(fbTypeName);
        taskNodeBuilder.add_link_node_id(fbInputNode.o);
        taskNodeBuilder.add_link_output_id(fbInputSlot.o);

        auto                                                  fbTaskNode = taskNodeBuilder.Finish().Union();

        std::vector<flatbuffers::Offset<flatbuffers::String>> outputTargetName(taskNode->GetOutputSize());
        for (size_t nOutputIdx = 0; nOutputIdx < taskNode->GetOutputSize(); ++nOutputIdx)
        {
            auto taskOutput = taskNode->GetOutput(nOutputIdx);
            if (!taskOutput) continue;
            auto renderOutput = taskOutput->DynamicCast<MRenderTaskNodeOutput>();
            if (!renderOutput) continue;

            outputTargetName[nOutputIdx] = fbb.CreateString(renderOutput->GetRenderTarget()->GetName().ToString());
        }

        auto                        fbOutputTargetName = fbb.CreateVector(outputTargetName);

        fbs::MRenderTaskNodeBuilder renderNodeBuilder(fbb);
        renderNodeBuilder.add_super(fbTaskNode.o);
        renderNodeBuilder.add_output_name(fbOutputTargetName.o);

        renderNodeArray.emplace_back(renderNodeBuilder.Finish().Union().o);
    }

    auto                     fbRenderNodeArray = fbb.CreateVector(renderNodeArray);

    fbs::MRenderGraphBuilder builder(fbb);
    builder.add_render_target(fbRenderTargetArray);
    builder.add_node_array(fbRenderNodeArray);

    flatbuffers::Offset<fbs::MRenderGraph> root = builder.Finish();
    fbb.Finish(root);

    output.resize(fbb.GetSize());
    memcpy(output.data(), fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));
}

MRenderTaskNode*
MDeferredRenderProgram::RegisterRenderNode(const MStringId& strTaskNodeName, const MString& strTaskNodeType)
{
    MRenderTaskNode* pRenderNode = m_renderGraph->FindTaskNode(strTaskNodeName);
    if (pRenderNode != nullptr) { return pRenderNode; }

    pRenderNode = m_renderGraph->RegisterTaskNode(strTaskNodeName, strTaskNodeType);
    if (pRenderNode)
    {
        pRenderNode->Initialize(GetEngine());
        if (auto pFramePropertyDecorator = pRenderNode->GetFramePropertyDecorator())
        {
            m_framePropertyAdapter->RegisterPropertyDecorator(pFramePropertyDecorator);
        }
    }
    return pRenderNode;
}