#include "MHBAORenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "MTransparentRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Render/RenderGraph/MRenderGraphSetting.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MHBAORenderNode, MBasicPostProcessRenderNode)

const MStringId            HbaoRadius       = MStringId("HBAO Radius");
const MStringId            HbaoNearestScale = MStringId("HBAO Nearest Scale");
const MStringId            HbaoOtherScale   = MStringId("HBAO Other Scale");
const MStringId            HbaoNDotVBias    = MStringId("HBAO NDotV Bias");

void                       MHBAORenderNode::Release() { Super::Release(); }

std::shared_ptr<MMaterial> MHBAORenderNode::CreateMaterial()
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

void MHBAORenderNode::RenderSetup(const MRenderInfo& info)
{
    auto setting = GetRenderGraph()->GetRenderGraphSetting()->GetValue<MVariantStruct>(GetNodeName());

    m_material->SetValue(MShaderPropertyName::HBAO_NEAREST_AO_SCALE, HbaoNearestScale);
    m_material->SetValue(MShaderPropertyName::HBAO_OTHER_AO_SCALE, HbaoOtherScale);
    m_material->SetValue(MShaderPropertyName::HBAO_NDOTV_BIAS, HbaoNDotVBias);
    m_material->SetValue(MShaderPropertyName::HBAO_RADIUS_UV_SQUARE_NEG_INV, -1.0f / (HbaoRadius * HbaoRadius));

    const float fFocalX    = info.m4ProjectionMatrix.m[0][0];
    const float fFocalY    = info.m4ProjectionMatrix.m[1][1];
    const float fInvFocalX = 1.0f / fFocalX;
    const float fInvFocalY = 1.0f / fFocalY;

    // View_xy = (uv * 2 - 1) * (invFocalX, invFocalY)
    Vector4     f4UVToView = Vector4(2.0f * fInvFocalX, -2.0f * fInvFocalY, -1.0f * fInvFocalX, 1.0f * fInvFocalY);
    m_material->SetValue(MShaderPropertyName::HBAO_UV_TO_VIEW, f4UVToView);


    const float fFocalLength = info.m4ProjectionMatrix.m[0][0];
    const float fRadiusUV    = 0.5f * HbaoRadius * fFocalLength;
    const float fRadiusPixel = fRadiusUV * static_cast<float>(info.f2ViewportSize.y);
    m_material->SetValue(MShaderPropertyName::HBAO_RADIUS_PIXEL, fRadiusPixel);
}

std::vector<MRenderTaskInputDesc> MHBAORenderNode::InitInputDesc()
{
    return {MRenderTaskNodeInput::CreateSample(
                    MRenderGraphName::GBuffer[1],
                    MRenderTaskNode::DefaultLinearSpaceFormat,
                    false
            ),
            MRenderTaskNodeInput::CreateSample(MRenderGraphName::DepthBuffer, METextureFormat::Depth, false)};
}

std::vector<MRenderTaskOutputDesc> MHBAORenderNode::InitOutputDesc()
{
    return {
            MRenderTaskNodeOutput::Create(
                    MRenderGraphName::TextureAO,
                    METextureFormat::UNorm_R8,
                    {true, MColor::Black_T}
            ),
    };
}