#include "MHBAOBlurRenderWork.h"

#include "MHBAORenderWork.h"
#include "MToneMappingRenderWork.h"
#include "Scene/MScene.h"

#include "RenderProgram/RenderGraph/MRenderGraph.h"

MStringId MHBAOBlurRenderWorkV::BlurOutput = MStringId("Hbao Blur Output V");

std::vector<MRenderTaskInputDesc> MHBAOBlurRenderWorkV::InitInputDesc()
{
	return {
		{ MHBAORenderWork::HBAOOutput, METextureBarrierStage::EPixelShaderSample }
	};
}

std::vector<MRenderTaskOutputDesc> MHBAOBlurRenderWorkV::InitOutputDesc()
{
	return {
		{ MHBAOBlurRenderWorkV::BlurOutput,  {true, MColor::Black_T, 0 }}
	};
}

MStringId MHBAOBlurRenderWorkH::BlurOutput = MStringId("Hbao Blur Output H");

std::vector<MRenderTaskInputDesc> MHBAOBlurRenderWorkH::InitInputDesc()
{
	return {
		{ MHBAOBlurRenderWorkV::BlurOutput, METextureBarrierStage::EPixelShaderSample }
	};
}

std::vector<MRenderTaskOutputDesc> MHBAOBlurRenderWorkH::InitOutputDesc()
{
	return {
		{ MHBAOBlurRenderWorkH::BlurOutput,  {true, MColor::Black_T, 0 }}
	};
}