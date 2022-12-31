#include "MPipeline.h"

VkPipeline MGraphicsPipeline::GetSubpassPipeline(size_t nSubPassIdx)
{
    if (nSubPassIdx < m_vSubpassPipeline.size())
    return m_vSubpassPipeline[nSubPassIdx];

    return VK_NULL_HANDLE;
}

MComputePipeline::MComputePipeline()
	: MPipeline()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
     m_vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
#endif
}