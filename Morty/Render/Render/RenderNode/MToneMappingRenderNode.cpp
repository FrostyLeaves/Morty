#include "MToneMappingRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MToneMappingRenderNode, MBasicPostProcessRenderNode)

std::shared_ptr<MMaterial> MToneMappingRenderNode::CreateMaterial()
{
    auto pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    auto pToneMappingMat = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Edge Detection");

    //TODO
    /*
    std::shared_ptr<MResource> pVertexShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pPixelShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
    pToneMappingMat->LoadShader(pVertexShader, MEShaderType::EVertex, MRenderGlobal::DEFAULT_VERTEX_ENTRY);
    pToneMappingMat->LoadShader(pPixelShader, MEShaderType::EPixel, MRenderGlobal::DEFAULT_PIXEL_ENTRY);
    pToneMappingMat->SetCullMode(MECullMode::ECullNone);
    */
    return MMaterial::CreateMaterial(pToneMappingMat);
    
}

std::vector<MRenderTaskInputDesc> MToneMappingRenderNode::InitInputDesc()
{
    return {MRenderTaskNodeInput::CreateSample(
            MRenderGraphName::ColorBuffer,
            MRenderTaskNode::DefaultLinearSpaceFormat,
            false
    )};
}

std::vector<MRenderTaskOutputDesc> MToneMappingRenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::ToneMapping,
                    METextureFormat::UNorm_RGBA8,
                    {true, MColor::Black_T}
            ),
    };
}