#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/MMath.h"

namespace morty
{

struct MBufferRHI;
class MPipeline;
class MRenderPass;
class MGraphicsPipeline;
class MShaderPropertyBlock;

enum class METextureBarrierStage
{
    EUnknow = 0,
    EPixelShaderSample,
    EPixelShaderWrite,
    EComputeShaderWrite,
    EComputeShaderRead,
    EShadingRateMask,
};

struct MORTY_API MRenderPassStage {
    MRenderPass* pRenderPass = nullptr;
    uint32_t     nSubpassIdx = 0;
};

struct IPipelineCmd {
    size_t type = 0;
};

template<size_t CmdType> struct MPipelineCmd : public IPipelineCmd {
                  MPipelineCmd() { type = CmdType; }
    static size_t GetType() { return CmdType; }
};

struct MSetViewportCmd : public MPipelineCmd<1> {
    float x        = 0;
    float y        = 0;
    float width    = 1;
    float height   = 1;
    float minDepth = 0;
    float maxDepth = 1;
};

struct MSetScissorCmd : public MPipelineCmd<2> {
    float x      = 0;
    float y      = 0;
    float width  = 1;
    float height = 1;
};

struct MDrawMeshCmd : public MPipelineCmd<3> {
    const MBufferRHI* vertexBuffer = nullptr;
    const MBufferRHI* indexBuffer  = nullptr;
    uint32_t          vertexOffset = 0;
    uint32_t          indexOffset  = 0;
    uint32_t          indexCount   = 0;
};

struct MDrawIndexedIndirectCmd : public MPipelineCmd<4> {
    const MBufferRHI* vertexBuffer   = nullptr;
    const MBufferRHI* indexBuffer    = nullptr;
    const MBufferRHI* commandsBuffer = nullptr;
    size_t            offset         = 0;
    size_t            count          = 0;
};

struct MSetGraphPipelineCmd : public MPipelineCmd<5> {
    const MGraphicsPipeline* pipeline   = nullptr;
    const size_t             subPassIdx = 0;
};

struct MSetShaderPropertyBlockCmd : public MPipelineCmd<6> {
    const MPipeline*      pipeline           = nullptr;
    MShaderPropertyBlock* property           = nullptr;
    bool                  allocDescriptorSet = false;
};

struct MAddTextureBarrierCmd : public MPipelineCmd<7> {
    const std::vector<MTexture*> textures;
    const METextureBarrierStage  dstStage = METextureBarrierStage::EPixelShaderSample;
};

struct MNextSubPassCmd : public MPipelineCmd<8> {
};

struct MSetShadingRateCmd : public MPipelineCmd<9> {
    Vector2i                               shadingRate{};
    std::array<MEShadingRateCombinerOp, 2> combineOp{};
};

}// namespace morty
