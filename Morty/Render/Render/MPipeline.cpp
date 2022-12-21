#include "MPipeline.h"

VkPipeline MGraphicsPipeline::GetSubpassPipeline(size_t nSubPassIdx)
{
    if (nSubPassIdx < m_vSubpassPipeline.size())
    return m_vSubpassPipeline[nSubPassIdx];

    return VK_NULL_HANDLE;
}
