#include "MHBAOBlurRenderNode.h"

#include "MHBAORenderNode.h"
#include "MToneMappingRenderNode.h"
#include "Scene/MScene.h"

#include "Render/RenderGraph/MRenderGraph.h"

using namespace morty;

MStringId                         MHBAOBlurRenderNodeV::BlurOutput = MStringId("Hbao Blur V");

std::vector<MRenderTaskInputDesc> MHBAOBlurRenderNodeV::InitInputDesc()
{
    return {{MHBAORenderNode::HBAOOutput, METextureBarrierStage::EPixelShaderSample}};
}

std::vector<MRenderTaskOutputDesc> MHBAOBlurRenderNodeV::InitOutputDesc()
{
    return {{MHBAOBlurRenderNodeV::BlurOutput, {true, MColor::Black_T, 0}}};
}

MStringId                         MHBAOBlurRenderNodeH::BlurOutput = MStringId("Hbao Blur H");

std::vector<MRenderTaskInputDesc> MHBAOBlurRenderNodeH::InitInputDesc()
{
    return {{MHBAOBlurRenderNodeV::BlurOutput, METextureBarrierStage::EPixelShaderSample}};
}

std::vector<MRenderTaskOutputDesc> MHBAOBlurRenderNodeH::InitOutputDesc()
{
    return {{MHBAOBlurRenderNodeH::BlurOutput, {true, MColor::Black_T, 0}}};
}