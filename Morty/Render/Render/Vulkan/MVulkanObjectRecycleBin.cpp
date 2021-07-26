#include "MVulkanObjectRecycleBin.h"
#include "MVulkanDevice.h"

#if RENDER_GRAPHICS == MORTY_VULKAN


MVulkanObjectRecycleBin::MVulkanObjectRecycleBin(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

void MVulkanObjectRecycleBin::EmptyTrash()
{
#define M_VULKAN_DESTROY_CLEAR(VK_TYPE, VK_FUNC) \
	for (Vk##VK_TYPE& buffer : m_v##VK_TYPE)\
		VK_FUNC(device, buffer, nullptr);

	VkDevice device = m_pDevice->m_VkDevice;

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

	if (!m_vDescriptorSet.empty())
	{
		vkFreeDescriptorSets(device, m_pDevice->m_VkDescriptorPool, m_vDescriptorSet.size(), m_vDescriptorSet.data());
	}

	for (VkCommandBuffer commandBuffer : m_vCommandBuffer)
	{
		vkFreeCommandBuffers(device, m_pDevice->m_VkCommandPool, 1, &commandBuffer);
	}

	for (MemoryInfo& info : m_vDynamicUniformMemory)
	{
		m_pDevice->m_BufferPool.FreeDynamicUniformMemory(info);
	}
}

bool MVulkanObjectRecycleBin::Initialize()
{
	return true;
}

void MVulkanObjectRecycleBin::Release()
{
	EmptyTrash();
}

#endif