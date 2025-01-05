#include "MToneMappingRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MTransparentRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MToneMappingRenderNode, MBasicPostProcessRenderNode)

MMaterialTemplatePtr MToneMappingRenderNode::CreateMaterial()
{
    auto pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto pToneMappingMat = pResourceSystem->FindResource<MMaterialTemplate>("PostProcess Tone Mapping");
    if (pToneMappingMat == nullptr)
    {
        pToneMappingMat = pResourceSystem->CreateResource<MMaterialTemplate>("PostProcess Tone Mapping");
        std::shared_ptr<MResource> pVertexShader =
                pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
        std::shared_ptr<MResource> pPixelShader =
                pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mps");
        pToneMappingMat->LoadShader(pVertexShader);
        pToneMappingMat->LoadShader(pPixelShader);
        pToneMappingMat->SetCullMode(MECullMode::ECullNone);
    }

    return pToneMappingMat;
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