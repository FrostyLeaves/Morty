#include "MColorGradingRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderGraphSetting.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MColorGradingRenderNode, MBasicPostProcessRenderNode)

const MStringId            MColorGradingRenderNode::ColorGradingOutput = MStringId("Color Grading Output");

void                       MColorGradingRenderNode::Release() { Super::Release(); }

std::shared_ptr<MMaterial> MColorGradingRenderNode::CreateMaterial()
{
    MResourceSystem* resourceSystem = GetEngine()->GetSystem<MResourceSystem>();

    auto             material = resourceSystem->CreateResource<MMaterialTemplate>("Color Grading Material");

    //TODO
    /*
    std::shared_ptr<MResource> pVertexShader =
            resourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pPixelShader = resourceSystem->LoadResource("Shader/ColorGrading/color_garding.mps");
    material->LoadShader(pVertexShader, MEShaderType::EVertex, MRenderGlobal::DEFAULT_VERTEX_ENTRY);
    material->LoadShader(pPixelShader, MEShaderType::EPixel, MRenderGlobal::DEFAULT_PIXEL_ENTRY);
    material->SetCullMode(MECullMode::ECullNone);
    */

    return MMaterial::CreateMaterial(material);
}

void MColorGradingRenderNode::RenderSetup(const MRenderInfo& info)
{
    MORTY_UNUSED(info);

    if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName())) {}
}

std::vector<MRenderTaskInputDesc> MColorGradingRenderNode::InitInputDesc()
{
    return {
            //	{ MGBufferRenderNode::GBufferNormalRoughness, METextureBarrierStage::EPixelShaderSample },
    };
}

std::vector<MRenderTaskOutputDesc> MColorGradingRenderNode::InitOutputDesc()
{
    return {
            //	{ ColorGradingOutput, {true, MColor::Black_T }},
    };
}