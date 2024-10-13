#include "MColorGradingRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MTransparentRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
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
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto                       pMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("Color Grading Material");
    std::shared_ptr<MResource> pVertexShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/ColorGrading/color_garding.mps");
    pMaterial->LoadShader(pVertexShader);
    pMaterial->LoadShader(pPixelShader);
    pMaterial->SetCullMode(MECullMode::ECullNone);

    return MMaterial::CreateMaterial(pMaterial);
}

void MColorGradingRenderNode::RenderSetup(const MRenderInfo& info)
{
    MORTY_UNUSED(info);

    if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName())) {}
}

void MColorGradingRenderNode::RegisterSetting()
{
    MVariantStruct ColorGradingSetting;
    MVariantStructBuilder(ColorGradingSetting).Finish();

    GetRenderGraph()->GetRenderGraphSetting()->RegisterProperty(GetNodeName(), ColorGradingSetting);
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