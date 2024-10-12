#pragma once

#include "Utility/MRenderGlobal.h"
#include "RHI/Abstract/MTextureRHI.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

namespace morty
{

struct MORTY_API MTextureRHIVulkan : MTextureRHI {
public:
    VkFormat       vkTextureFormat      = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageLayout  vkImageLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage        vkTextureImage       = VK_NULL_HANDLE;
    VkDeviceMemory vkTextureImageMemory = VK_NULL_HANDLE;
    VkImageView    vkImageView          = VK_NULL_HANDLE;
    VkSampler      vkSampler            = VK_NULL_HANDLE;
};

}// namespace morty

#endif