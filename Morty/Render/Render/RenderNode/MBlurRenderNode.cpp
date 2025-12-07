#include "MBlurRenderNode.h"

#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Scene/MScene.h"

#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderGraphSetting.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MBlurRenderNode, MBasicPostProcessRenderNode)

const MStringId            BlurOffsetName    = MStringId("Gaussian Blur Offset");
const MStringId            BlurDirectionName = MStringId("Gaussian Blur Vertical");

std::shared_ptr<MMaterial> MBlurRenderNode::CreateMaterial()
{
    auto resourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto pBlurMaterial = resourceSystem->FindResource<MMaterialTemplate>("blur");

    //TODO
    /*
    if (pBlurMaterial == nullptr)
    {
        pBlurMaterial = resourceSystem->CreateResource<MMaterialTemplate>("blur");
        std::shared_ptr<MResource> pVertexShader =
                resourceSystem->LoadResource("Shader/PostProcess/gaussian_blur.mvs");
        std::shared_ptr<MResource> pPixelShader = resourceSystem->LoadResource("Shader/PostProcess/gaussian_blur.mps");
        pBlurMaterial->LoadShader(pVertexShader, MEShaderType::EVertex, MRenderGlobal::DEFAULT_VERTEX_ENTRY);
        pBlurMaterial->LoadShader(pPixelShader, MEShaderType::EPixel, MRenderGlobal::DEFAULT_PIXEL_ENTRY);
        pBlurMaterial->SetCullMode(MECullMode::ECullNone);
    }
    */
    return MMaterial::CreateMaterial(pBlurMaterial);
}

void MBlurRenderNode::RenderSetup(const MRenderInfo& info)
{
    if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName()))
    {
        auto          setting = GetRenderGraph()->GetRenderGraphSetting()->GetValue<MVariantStruct>(GetNodeName());

        const float   fOffset   = setting.GetVariant<float>(BlurOffsetName);
        const bool    bVertical = setting.GetVariant<bool>(BlurDirectionName);

        const Vector2 f2Offset =
                Vector2(bVertical ? 0 : (fOffset / info.viewportRect.width),
                        bVertical ? (fOffset / info.viewportRect.height) : 0);

        m_material->SetValue(MShaderPropertyName::POSTPROCESS_BLUR_OFFSET, f2Offset);
    }
}

void MBlurRenderNode::RegisterSetting()
{
    MVariantStruct blurSetting;
    MVariantStructBuilder(blurSetting)
            .AppendVariant(BlurOffsetName, 2.0f)
            .AppendVariant(BlurDirectionName, m_vertical)
            .Finish();

    GetRenderGraph()->GetRenderGraphSetting()->RegisterProperty(GetNodeName(), blurSetting);
}

std::vector<MRenderTaskInputDesc> MBlurRenderNode::InitInputDesc()
{
    MORTY_ASSERT(false);
    return {};
}

std::vector<MRenderTaskOutputDesc> MBlurRenderNode::InitOutputDesc()
{
    MORTY_ASSERT(false);
    return {};
}