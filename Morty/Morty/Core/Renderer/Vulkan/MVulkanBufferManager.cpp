#include "MVulkanBufferManager.h"
#include "MVulkanDevice.h"

#if RENDER_GRAPHICS == MORTY_VULKAN


MVulkanBufferManager::MVulkanBufferManager(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

void MVulkanBufferManager::FrameFinished(const uint32_t& unFrameIndex)
{
	VkDevice device = m_pDevice->m_VkDevice;

	for (VkBuffer& buffer : m_vBuffer[unFrameIndex])
		vkDestroyBuffer(device, buffer, nullptr);

	m_vBuffer[unFrameIndex].clear();

	for(VkDeviceMemory& memory : m_vDeviceMemory[unFrameIndex])
		vkFreeMemory(device, memory, nullptr);

	m_vDeviceMemory[unFrameIndex].clear();

	for (VkFramebuffer& buffer : m_vFrameBuffer[unFrameIndex])
		vkDestroyFramebuffer(device, buffer, nullptr);

	m_vFrameBuffer[unFrameIndex].clear();

	for (VkImageView& view : m_vImageView[unFrameIndex])
		vkDestroyImageView(device, view, nullptr);

	m_vImageView[unFrameIndex].clear();

	for (VkImage& image : m_vImage[unFrameIndex])
		vkDestroyImage(device, image, nullptr);

	m_vImage[unFrameIndex].clear();

	for (VkRenderPass& pass : m_vRenderPass[unFrameIndex])
		vkDestroyRenderPass(device, pass, nullptr);

	m_vRenderPass[unFrameIndex].clear();
}

bool MVulkanBufferManager::GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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

void MVulkanBufferManager::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	vkDestroyBuffer(m_pDevice->m_VkDevice, buffer, nullptr);
	vkFreeMemory(m_pDevice->m_VkDevice, bufferMemory, nullptr);
}



void MVulkanBufferManager::DestroyBufferLater(const uint32_t& unFrameIndex, VkBuffer& buffer)
{
	m_vBuffer[unFrameIndex].push_back(buffer);
}

void MVulkanBufferManager::DestroyDeviceMemoryLater(const uint32_t& unFrameIndex, VkDeviceMemory& memory)
{
	m_vDeviceMemory[unFrameIndex].push_back(memory);
}

void MVulkanBufferManager::DestroyFrameBufferLater(const uint32_t& unFrameIndex, VkFramebuffer& buffer)
{
	m_vFrameBuffer[unFrameIndex].push_back(buffer);
}

void MVulkanBufferManager::DestroyImageLater(const uint32_t& unFrameIndex, VkImage& image)
{
	m_vImage[unFrameIndex].push_back(image);
}

void MVulkanBufferManager::DestroyImageViewLater(const uint32_t& unFrameIndex, VkImageView& view)
{
	m_vImageView[unFrameIndex].push_back(view);
}

void MVulkanBufferManager::DestroyRenderPassLater(const uint32_t& unFrameIndex, VkRenderPass& pass)
{
	m_vRenderPass[unFrameIndex].push_back(pass);
}

#endif