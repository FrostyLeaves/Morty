#include "MPipeline.h"

using namespace morty;

VkPipeline MGraphicsPipeline::GetSubpassPipeline(size_t nSubPassIdx)
{
    if (nSubPassIdx < m_subpassPipeline.size()) return m_subpassPipeline[nSubPassIdx];

    return VK_NULL_HANDLE;
}

MComputePipeline::MComputePipeline()
    : MPipeline()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
#endif
}