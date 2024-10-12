#include "MColorGradingRenderWork.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderWork.h"
#include "MTransparentRenderWork.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "RenderProgram/RenderGraph/MRenderGraphSetting.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MColorGradingRenderWork, MBasicPostProcessRenderWork)

const MStringId            MColorGradingRenderWork::ColorGradingOutput = MStringId("Color Grading Output");

void                       MColorGradingRenderWork::Release() { Super::Release(); }

std::shared_ptr<MMaterial> MColorGradingRenderWork::CreateMaterial()
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

void MColorGradingRenderWork::RenderSetup(const MRenderInfo& info)
{
    MORTY_UNUSED(info);

    if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName())) {}
}

void MColorGradingRenderWork::RegisterSetting()
{
    MVariantStruct ColorGradingSetting;
    MVariantStructBuilder(ColorGradingSetting).Finish();

    GetRenderGraph()->GetRenderGraphSetting()->RegisterProperty(GetNodeName(), ColorGradingSetting);
}

std::vector<MRenderTaskInputDesc> MColorGradingRenderWork::InitInputDesc()
{
    return {
            //	{ MGBufferRenderWork::GBufferNormalRoughness, METextureBarrierStage::EPixelShaderSample },
    };
}

std::vector<MRenderTaskOutputDesc> MColorGradingRenderWork::InitOutputDesc()
{
    return {
            //	{ ColorGradingOutput, {true, MColor::Black_T }},
    };
}