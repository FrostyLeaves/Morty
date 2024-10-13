#include "MToneMappingRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MTransparentRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MToneMappingRenderNode, MBasicPostProcessRenderNode)

const MStringId            MToneMappingRenderNode::ToneMappingResult = MStringId("Tone Mapping Output");

std::shared_ptr<MMaterial> MToneMappingRenderNode::CreateMaterial()
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto             pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");
    std::shared_ptr<MResource> pVertexShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pPixelShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
    pEdgeMaterial->LoadShader(pVertexShader);
    pEdgeMaterial->LoadShader(pPixelShader);
    pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

    return MMaterial::CreateMaterial(pEdgeMaterial);
}

std::vector<MRenderTaskInputDesc> MToneMappingRenderNode::InitInputDesc()
{
    return {
            {MTransparentRenderNode::BackBufferOutput, METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MToneMappingRenderNode::InitOutputDesc()
{
    return {
            {ToneMappingResult, {true, MColor::Black_T}},
    };
}