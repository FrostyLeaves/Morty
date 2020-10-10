#include "MVulkanObjectDestructor.h"
#include "MVulkanDevice.h"

#define M_VULKAN_DESTROY_CLEAR(VK_TYPE, VK_FUNC) \
	for (Vk##VK_TYPE& buffer : m_v##VK_TYPE[m_unSafeIdx])\
		VK_FUNC(device, buffer, nullptr);\
	m_v##VK_TYPE[m_unSafeIdx].clear();\

#if RENDER_GRAPHICS == MORTY_VULKAN


MVulkanObjectDestructor::MVulkanObjectDestructor(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
	, m_VkDescriptorPool(VK_NULL_HANDLE)
	, m_VkDefaultSampler(VK_NULL_HANDLE)
	, m_VkLessEqualSampler(VK_NULL_HANDLE)
	, m_unSafeIdx(0)
{

}

void MVulkanObjectDestructor::FrameFinished(const uint32_t& unFrameIndex)
{
	VkDevice device = m_pDevice->m_VkDevice;

	if (unFrameIndex != M_BUFFER_NUM - 1)
		return;

	m_unSafeIdx = (m_unSafeIdx + 1) % 2;

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

	for (auto& set : m_vDescriptorSets[m_unSafeIdx])
		vkFreeDescriptorSets(device, m_VkDescriptorPool, set.size(), set.data());
	m_vDescriptorSets[m_unSafeIdx].clear();

	for (VkCommandBuffer commandBuffer : m_vCommandBuffer[m_unSafeIdx])
		vkFreeCommandBuffers(device, m_pDevice->m_VkCommandPool, 1, &commandBuffer);
	m_vCommandBuffer[m_unSafeIdx].clear();
}

bool MVulkanObjectDestructor::Initialize()
{
	if (!InitDescriptorPool())
		return false;

	if (!InitSampler())
		return false;

	return true;
}

void MVulkanObjectDestructor::Release()
{
	//Release All Vk Object.
	FrameFinished(M_BUFFER_NUM - 1);
	FrameFinished(M_BUFFER_NUM - 1);

	vkDestroyDescriptorPool(m_pDevice->m_VkDevice, m_VkDescriptorPool, nullptr);

	vkDestroySampler(m_pDevice->m_VkDevice, m_VkDefaultSampler, nullptr);

	vkDestroySampler(m_pDevice->m_VkDevice, m_VkLessEqualSampler, nullptr);
}

bool MVulkanObjectDestructor::GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_pDevice->m_VkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_pDevice->m_VkDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_pDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (-1 == allocInfo.memoryTypeIndex)
	{
		vkDestroyBuffer(m_pDevice->m_VkDevice, buffer, nullptr);
		return false;
	}

	if (vkAllocateMemory(m_pDevice->m_VkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_pDevice->m_VkDevice, buffer, nullptr);
		return false;
	}

	vkBindBufferMemory(m_pDevice->m_VkDevice, buffer, bufferMemory, 0);

	return true;
}

void MVulkanObjectDestructor::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	vkDestroyBuffer(m_pDevice->m_VkDevice, buffer, nullptr);
	vkFreeMemory(m_pDevice->m_VkDevice, bufferMemory, nullptr);
}

bool MVulkanObjectDestructor::InitDescriptorPool()
{
	uint32_t unSwapChainNum = 500;

	std::vector<VkDescriptorPoolSize> vPoolSize(5);
	vPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vPoolSize[0].descriptorCount = unSwapChainNum;

	vPoolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	vPoolSize[1].descriptorCount = unSwapChainNum;

	vPoolSize[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	vPoolSize[2].descriptorCount = unSwapChainNum;

	vPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	vPoolSize[3].descriptorCount = unSwapChainNum;

	vPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	vPoolSize[4].descriptorCount = unSwapChainNum;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = vPoolSize.size();
	poolInfo.pPoolSizes = vPoolSize.data();

	poolInfo.maxSets = unSwapChainNum;

	if (vkCreateDescriptorPool(m_pDevice->m_VkDevice, &poolInfo, nullptr, &m_VkDescriptorPool) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool MVulkanObjectDestructor::InitSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(m_pDevice->m_VkDevice, &samplerInfo, nullptr, &m_VkDefaultSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	samplerInfo.compareEnable = VK_TRUE;
	samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	if (vkCreateSampler(m_pDevice->m_VkDevice, &samplerInfo, nullptr, &m_VkLessEqualSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return true;
}


void MVulkanObjectDestructor::DestroyDescriptorSetsLater(std::vector<VkDescriptorSet>& vDescriptorSets)
{
	m_vDescriptorSets[m_unSafeIdx].push_back(std::move(vDescriptorSets));
}

#endif