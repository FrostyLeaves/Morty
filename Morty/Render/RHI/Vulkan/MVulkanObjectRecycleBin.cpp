#include "MVulkanObjectRecycleBin.h"
#include "RHI/Vulkan/MVulkanDevice.h"

using namespace morty;

#if RENDER_GRAPHICS == MORTY_VULKAN


MVulkanObjectRecycleBin::MVulkanObjectRecycleBin(MVulkanDevice* pDevice)
    : m_device(pDevice)
{}

void MVulkanObjectRecycleBin::EmptyTrash()
{
#define M_VULKAN_DESTROY_CLEAR(VK_TYPE, VK_FUNC)                                                                       \
    for (Vk##VK_TYPE & buffer: m_##VK_TYPE) VK_FUNC(device, buffer, nullptr);                                          \
    m_##VK_TYPE.clear();

    VkDevice device = m_device->m_vkDevice;

    M_VULKAN_DESTROY_CLEAR(Buffer, vkDestroyBuffer);
    M_VULKAN_DESTROY_CLEAR(DeviceMemory, vkFreeMemory);
    M_VULKAN_DESTROY_CLEAR(Framebuffer, vkDestroyFramebuffer);
    M_VULKAN_DESTROY_CLEAR(ImageView, vkDestroyImageView);
    M_VULKAN_DESTROY_CLEAR(Image, vkDestroyImage);
    M_VULKAN_DESTROY_CLEAR(RenderPass, vkDestroyRenderPass);
    M_VULKAN_DESTROY_CLEAR(PipelineLayout, vkDestroyPipelineLayout);
    M_VULKAN_DESTROY_CLEAR(DescriptorSetLayout, vkDestroyDescriptorSetLayout);
    M_VULKAN_DESTROY_CLEAR(ShaderModule, vkDestroyShaderModule);
    M_VULKAN_DESTROY_CLEAR(Pipeline, vkDestroyPipeline);
    M_VULKAN_DESTROY_CLEAR(Fence, vkDestroyFence);
    M_VULKAN_DESTROY_CLEAR(Semaphore, vkDestroySemaphore);
    M_VULKAN_DESTROY_CLEAR(Event, vkDestroyEvent);
    M_VULKAN_DESTROY_CLEAR(Sampler, vkDestroySampler);

    for (VkFence& buffer: m_Fence) vkDestroyFence(device, buffer, nullptr);
    m_Fence.clear();

    if (!m_DescriptorSet.empty())
        vkFreeDescriptorSets(
                device,
                m_device->m_vkDescriptorPool,
                static_cast<uint32_t>(m_DescriptorSet.size()),
                m_DescriptorSet.data()
        );
    m_DescriptorSet.clear();

    for (MemoryInfo& info: m_dynamicUniformMemory) m_device->m_BufferPool.FreeDynamicUniformMemory(info);
    m_dynamicUniformMemory.clear();
}

bool MVulkanObjectRecycleBin::Initialize() { return true; }

void MVulkanObjectRecycleBin::Release() { EmptyTrash(); }

#endif
