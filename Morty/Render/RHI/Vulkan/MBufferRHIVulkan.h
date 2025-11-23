#pragma once

#include "Utility/MRenderGlobal.h"
#include "RHI/Abstract/MBufferRHI.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

namespace morty
{

struct MORTY_API MBufferRHIVulkan : public MBufferRHI {
    VkBuffer       vkBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
};

}// namespace morty

#endif