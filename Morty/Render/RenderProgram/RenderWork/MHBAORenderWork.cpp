#include "MHBAORenderWork.h"

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

MORTY_CLASS_IMPLEMENT(MHBAORenderWork, MBasicPostProcessRenderWork)

const MStringId            MHBAORenderWork::HBAOOutput = MStringId("HBAO Output");

const MStringId            HbaoRadius       = MStringId("HBAO Radius");
const MStringId            HbaoNearestScale = MStringId("HBAO Nearest Scale");
const MStringId            HbaoOtherScale   = MStringId("HBAO Other Scale");
const MStringId            HbaoNDotVBias    = MStringId("HBAO NDotV Bias");

void                       MHBAORenderWork::Release() { Super::Release(); }

std::shared_ptr<MMaterial> MHBAORenderWork::CreateMaterial()
{
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    auto                       pEdgeMaterial = pResourceSystem->CreateResource<MMaterialTemplate>("HBAO Generator");
    std::shared_ptr<MResource> pVertexShader =
            pResourceSystem->LoadResource("Shader/PostProcess/post_process_basic.mvs");
    std::shared_ptr<MResource> pPixelShader = pResourceSystem->LoadResource("Shader/AO/hbao.mps");
    pEdgeMaterial->LoadShader(pVertexShader);
    pEdgeMaterial->LoadShader(pPixelShader);
    pEdgeMaterial->SetCullMode(MECullMode::ECullNone);

    return MMaterial::CreateMaterial(pEdgeMaterial);
}

void MHBAORenderWork::RenderSetup(const MRenderInfo& info)
{
    auto        setting = GetRenderGraph()->GetRenderGraphSetting()->GetValue<MVariantStruct>(GetNodeName());
    const float fRadius = setting.GetVariant<float>(HbaoRadius);

    if (GetRenderGraph()->GetRenderGraphSetting()->IsDirty(GetNodeName()))
    {
        const float fNearestAoScale = setting.GetVariant<float>(HbaoNearestScale);
        const float fOtherAoScale   = setting.GetVariant<float>(HbaoOtherScale);
        const float fNDotVBias      = setting.GetVariant<float>(HbaoNDotVBias);

        m_material->SetValue(MShaderPropertyName::HBAO_NEAREST_AO_SCALE, fNearestAoScale);
        m_material->SetValue(MShaderPropertyName::HBAO_OTHER_AO_SCALE, fOtherAoScale);
        m_material->SetValue(MShaderPropertyName::HBAO_NDOTV_BIAS, fNDotVBias);
        m_material->SetValue(MShaderPropertyName::HBAO_RADIUS_UV_SQUARE_NEG_INV, -1.0f / (fRadius * fRadius));
    }

    const float fFocalX    = info.m4ProjectionMatrix.m[0][0];
    const float fFocalY    = info.m4ProjectionMatrix.m[1][1];
    const float fInvFocalX = 1.0 / fFocalX;
    const float fInvFocalY = 1.0 / fFocalY;

    // View_xy = (uv * 2 - 1) * (invFocalX, invFocalY)
    Vector4     f4UVToView = Vector4(2.0 * fInvFocalX, -2.0 * fInvFocalY, -1.0 * fInvFocalX, 1.0 * fInvFocalY);
    m_material->SetValue(MShaderPropertyName::HBAO_UV_TO_VIEW, f4UVToView);


    const float fFocalLength = info.m4ProjectionMatrix.m[0][0];
    const float fRadiusUV    = 0.5f * fRadius * fFocalLength;
    const float fRadiusPixel = fRadiusUV * info.f2ViewportSize.y;
    m_material->SetValue(MShaderPropertyName::HBAO_RADIUS_PIXEL, fRadiusPixel);
}

void MHBAORenderWork::RegisterSetting()
{
    MVariantStruct HbaoSetting;
    MVariantStructBuilder(HbaoSetting)
            .AppendVariant(HbaoRadius, 2.0f)
            .AppendVariant(HbaoNearestScale, 1.0f)
            .AppendVariant(HbaoOtherScale, 1.0f)
            .AppendVariant(HbaoNDotVBias, 0.2f)
            .Finish();

    GetRenderGraph()->GetRenderGraphSetting()->RegisterProperty(GetNodeName(), HbaoSetting);
}

std::vector<MRenderTaskInputDesc> MHBAORenderWork::InitInputDesc()
{
    return {
            {MGBufferRenderWork::GBufferNormalRoughness, METextureBarrierStage::EPixelShaderSample},
            {MGBufferRenderWork::GBufferDepthBufferOutput, METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MHBAORenderWork::InitOutputDesc()
{
    return {
            {HBAOOutput, {true, MColor::Black_T}},
    };
}