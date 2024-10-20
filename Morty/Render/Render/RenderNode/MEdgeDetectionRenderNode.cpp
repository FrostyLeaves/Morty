#include "MEdgeDetectionRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEdgeDetectionRenderNode, MBasicPostProcessRenderNode)

const MStringId            MEdgeDetectionRenderNode::EdgeDetectionResult = MStringId("Edge Detection");

std::shared_ptr<MMaterial> MEdgeDetectionRenderNode::CreateMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto             pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");
    std::shared_ptr<MResource> pVertexShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pEdgePixelShader =
            pResourceSystem->LoadResource("Shader/PostProcess/sobel_edge_detection.mps");
    pEdgeMaterial->LoadShader(pVertexShader);
    pEdgeMaterial->LoadShader(pEdgePixelShader);
    pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

    return MMaterial::CreateMaterial(pEdgeMaterial);
}

std::vector<MRenderTaskInputDesc> MEdgeDetectionRenderNode::InitInputDesc()
{
    return {
            {MForwardRenderNode::BackBufferOutput,
             MRenderTaskNode::DefaultLinearSpaceFormat,
             METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MEdgeDetectionRenderNode::InitOutputDesc()
{
    return {
            {EdgeDetectionResult, METextureFormat::UNorm_RGBA8, {true, MColor::Black_T}},
    };
}