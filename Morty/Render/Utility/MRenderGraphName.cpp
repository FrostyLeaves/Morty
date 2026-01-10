#include "MRenderGraphName.h"

using namespace morty;


const MStringId MRenderGraphName::ColorBuffer   = MStringId("Color Buffer");
const MStringId MRenderGraphName::DepthBuffer   = MStringId("Depth Buffer");
const MStringId MRenderGraphName::EdgeDetection = MStringId("Edge Detection");
const MStringId MRenderGraphName::ShadowMap     = MStringId("Shadow Map");
const MStringId MRenderGraphName::ToneMapping   = MStringId("Tone Mapping");
const MStringId MRenderGraphName::Voxelizer     = MStringId("Voxelizer Buffer");
const MStringId MRenderGraphName::TextureVRS    = MStringId("VRS");
const MStringId MRenderGraphName::TextureAO     = MStringId("AO");

const MStringId MRenderGraphName::TransparentFront = MStringId("Transparent Front");
const MStringId MRenderGraphName::TransparentBack  = MStringId("Transparent Back");
const MStringId MRenderGraphName::Renderer         = MStringId("Renderer");

const MStringId MRenderGraphName::GBuffer[3] = {
        MStringId("Albedo Metallic"),
        MStringId("Normal Roughness"),
        MStringId("Position AO"),
};