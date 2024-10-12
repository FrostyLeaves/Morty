/**
 * @File         MVulkanObjectDestructor
 * 
 * @Created      2020-07-07 21:06:34
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MMemoryPool.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanWrapper.h"

namespace morty
{

class MVulkanDevice;
class MORTY_API MVulkanObjectRecycleBin
{
public:
    MVulkanObjectRecycleBin(MVulkanDevice* pDevice);

    ~MVulkanObjectRecycleBin() {}

public:
    bool Initialize();

    void Release();

public:
    void EmptyTrash();

public:
#define M_VULKAN_DESTROY_LATER_FUNC(VK_TYPE)                                                                           \
    std::vector<Vk##VK_TYPE> m_##VK_TYPE;                                                                              \
    void                     Destroy##VK_TYPE##Later(Vk##VK_TYPE buffer)                                               \
    {                                                                                                                  \
        if (VK_NULL_HANDLE != buffer) { m_##VK_TYPE.push_back(buffer); }                                               \
    }

    M_VULKAN_DESTROY_LATER_FUNC(Buffer);
    M_VULKAN_DESTROY_LATER_FUNC(Framebuffer);
    M_VULKAN_DESTROY_LATER_FUNC(DeviceMemory);
    M_VULKAN_DESTROY_LATER_FUNC(ImageView);
    M_VULKAN_DESTROY_LATER_FUNC(Image);
    M_VULKAN_DESTROY_LATER_FUNC(RenderPass);
    M_VULKAN_DESTROY_LATER_FUNC(PipelineLayout);
    M_VULKAN_DESTROY_LATER_FUNC(DescriptorSetLayout);
    M_VULKAN_DESTROY_LATER_FUNC(ShaderModule);
    M_VULKAN_DESTROY_LATER_FUNC(Pipeline);
    M_VULKAN_DESTROY_LATER_FUNC(Fence);
    M_VULKAN_DESTROY_LATER_FUNC(Semaphore);
    M_VULKAN_DESTROY_LATER_FUNC(Event);
    M_VULKAN_DESTROY_LATER_FUNC(Sampler);
    M_VULKAN_DESTROY_LATER_FUNC(DescriptorSet);

    std::vector<MemoryInfo> m_dynamicUniformMemory;

    void           DestroyDynamicUniformMemoryLater(MemoryInfo& info) { m_dynamicUniformMemory.push_back(info); }

    MVulkanDevice* m_device;
};

}// namespace morty

#endif
