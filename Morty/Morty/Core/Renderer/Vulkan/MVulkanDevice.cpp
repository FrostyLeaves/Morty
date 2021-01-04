#include "MVulkanDevice.h"
#include "MMesh.h"
#include "MTexture.h"
#include "MResource.h"
#include "MFileHelper.h"
#include "Shader/MShader.h"
#include "MRenderStructure.h"
#include "Shader/MShaderParam.h"
#include "Shader/MShaderBuffer.h"

#include <set>
#include <array>
#include <assert.h>

#define VALUE_MAX(a, b)(a > b ? a : b)

#if RENDER_GRAPHICS == MORTY_VULKAN


#include "MVulkanRenderTarget.h"

#ifdef MORTY_WIN
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#ifdef MORTY_ANDROID
#include "vulkan/vulkan_android.h"
#endif

const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME
};


#define GET_INSTANCE_PROC_ADDR(inst, entry)                              \
  {                                                                      \
    entry = (PFN_vk##entry)vkGetInstanceProcAddr(inst, "vk" #entry); \
    if (!entry) {\
	MLogManager::GetInstance()->Error("vkGetInstanceProcAddr failed to find vk" #entry); \
		return false; \
	} \
  }

#define GET_DEVICE_PROC_ADDR(dev, entry)                              \
  {                                                                   \
    entry = (PFN_vk##entry)vkGetDeviceProcAddr(dev, "vk" #entry); \
    if (!entry) {                                                 \
      MLogManager::GetInstance()->Error("vkGetDeviceProcAddr failed to find vk" #entry);    \
		return false; \
	} \
  }


MVulkanDevice::MVulkanDevice()
	: MIDevice()
	, m_VkInstance(VK_NULL_HANDLE)
	, m_VkPhysicalDevice(VK_NULL_HANDLE)
	, m_VkPhysicalDeviceProperties({})
	, m_VkDevice(VK_NULL_HANDLE)
	, m_VkGraphicsQueue(VK_NULL_HANDLE)
	, m_ObjectDestructor(this)
	, m_PipelineManager(this)
	, m_DynamicUniformBufferPool(this)
{

}

MVulkanDevice::~MVulkanDevice()
{

}

bool MVulkanDevice::Initialize()
{
#ifdef MORTY_ANDROID
	if (!InitVulkan())
		return false;
#endif

	if (!InitVulkanInstance())
		return false;

	if (!InitPhysicalDevice())
		return false;

	if (!InitLogicalDevice())
		return false;

	if (!InitCommandPool())
		return false;
	
	if (!m_ObjectDestructor.Initialize())
		return false;

	if (!m_DynamicUniformBufferPool.Initialize())
		return false;

	if (!InitDefaultTexture())
		return false;

	if (!InitDepthFormat())
		return false;

	GET_INSTANCE_PROC_ADDR(m_VkInstance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(m_VkInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_VkInstance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_VkInstance, GetPhysicalDeviceSurfacePresentModesKHR);

	GET_DEVICE_PROC_ADDR(m_VkDevice, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_VkDevice, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_VkDevice, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(m_VkDevice, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(m_VkDevice, QueuePresentKHR);


	m_ShaderCompiler.Initialize();

	return true;
}

void MVulkanDevice::Release()
{
	m_WhiteTexture.DestroyTexture(this);

	m_PipelineManager.Release();
	m_DynamicUniformBufferPool.Release();

	m_ObjectDestructor.Release();

	vkDestroyCommandPool(m_VkDevice, m_VkCommandPool, nullptr);

	vkDestroyDevice(m_VkDevice, nullptr);

	vkDestroyInstance(m_VkInstance, NULL);
}

int MVulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &memProperties);

	for (int i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return M_INVALID_INDEX;
}

int MVulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (int i = 0; i < candidates.size(); ++i)
	{
		const VkFormat& format = candidates[i];

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return i;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return i;
		}
	}

	return M_INVALID_INDEX;
}

VkBool32 MVulkanDevice::FormatIsFilterable(VkFormat format, VkImageTiling tiling)
{
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &formatProps);

	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	return false;
}

VkFormat MVulkanDevice::GetFormat(const METextureLayout& layout)
{
	switch (layout)
	{
	case METextureLayout::ER32:
		return VK_FORMAT_R32_SFLOAT;
		break;

	case METextureLayout::ERGBA8:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;

	case METextureLayout::ERGBA16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
		break;

	default:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;
	}

	return VK_FORMAT_R8G8B8A8_SRGB;
}

bool MVulkanDevice::InitDepthFormat()
{
	std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

	int index = FindSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	if (index == M_INVALID_INDEX)
		return false;

	m_VkDepthTextureFormat = formats[index];
	return true;
}

void MVulkanDevice::GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable /*= false*/)
{
	if (*ppVertexBuffer)
		DestroyBuffer(ppVertexBuffer);

	void* data = nullptr;

	VkDeviceSize bufferSize = pMesh->GetVerticesLength() * pMesh->GetVertexStructSize();
	VkDeviceSize indexBufferSize = sizeof(uint32_t) * pMesh->GetIndicesLength();

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	if (bModifiable)
	{
		m_ObjectDestructor.GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
		vkMapMemory(m_VkDevice, vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, vertexBufferMemory);

		m_ObjectDestructor.GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT , indexBuffer, indexBufferMemory);
		vkMapMemory(m_VkDevice, indexBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, indexBufferMemory);
	}
	else
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_ObjectDestructor.GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		
		m_ObjectDestructor.GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		m_ObjectDestructor.DestroyBuffer(stagingBuffer, stagingBufferMemory);


		VkBuffer stagingIdxBuffer;
		VkDeviceMemory stagingIdxBufferMemory;
		m_ObjectDestructor.GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIdxBuffer, stagingIdxBufferMemory);

		vkMapMemory(m_VkDevice, stagingIdxBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, stagingIdxBufferMemory);

		m_ObjectDestructor.GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingIdxBuffer, indexBuffer, indexBufferSize);

		m_ObjectDestructor.DestroyBuffer(stagingIdxBuffer, stagingIdxBufferMemory);
	}

	MVertexBuffer* pBuffer = new MVertexBuffer();
	pBuffer->m_VkVertexBuffer = vertexBuffer;
	pBuffer->m_VkVertexBufferMemory = vertexBufferMemory;
	pBuffer->m_VkIndexBuffer = indexBuffer;
	pBuffer->m_VkIndexBufferMemory = indexBufferMemory;

	*ppVertexBuffer = pBuffer;
}

void MVulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndCommands(commandBuffer);
}

void MVulkanDevice::CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount)
{
	VkCommandBuffer commandBuffer = BeginCommands();


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = unCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	EndCommands(commandBuffer);
}

void MVulkanDevice::DestroyBuffer(MVertexBuffer** ppVertexBuffer)
{
	if (*ppVertexBuffer)
	{
		m_ObjectDestructor.DestroyBufferLater((*ppVertexBuffer)->m_VkVertexBuffer);
		m_ObjectDestructor.DestroyDeviceMemoryLater((*ppVertexBuffer)->m_VkVertexBufferMemory);
		m_ObjectDestructor.DestroyBufferLater((*ppVertexBuffer)->m_VkIndexBuffer);
		m_ObjectDestructor.DestroyDeviceMemoryLater((*ppVertexBuffer)->m_VkIndexBufferMemory);
		delete *ppVertexBuffer;
		*ppVertexBuffer = nullptr;
	}
}

void MVulkanDevice::TransitionImageLayout(VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout,VkImageSubresourceRange subresourceRange, VkCommandBuffer buffer/* = VK_NULL_HANDLE*/)
{
	VkCommandBuffer commandBuffer = buffer ? buffer : BeginCommands();

	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask
			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		assert(false);
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask
			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask
				= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		assert(false);
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, destStageFlags, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemoryBarrier);

	if (!buffer)
	{
		EndCommands(commandBuffer);
	}
}

VkImageView MVulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_VkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		VK_NULL_HANDLE;
	}

	return imageView;
}

void MVulkanDevice::CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = VALUE_MAX(unWidth, 1);
	imageInfo.extent.height = VALUE_MAX(unHeight, 1);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
}

void MVulkanDevice::CreateImageCube(const uint32_t& unWidth, const uint32_t& unHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { unWidth, unHeight, 1 };
	imageCreateInfo.usage = usage;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	vkCreateImage(m_VkDevice, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
}

VkCommandBuffer MVulkanDevice::BeginCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void MVulkanDevice::EndCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_VkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	vkQueueWaitIdle(m_VkGraphicsQueue);	//��ͣӦ�ó���ֱ������ύ�������е����й���

	vkFreeCommandBuffers(m_VkDevice, m_VkCommandPool, 1, &commandBuffer);
}

void MVulkanDevice::UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh)
{
	void* data = nullptr;
	uint32_t unSize = pMesh->GetVerticesLength() * pMesh->GetVertexStructSize();
	vkMapMemory(m_VkDevice, (*ppVertexBuffer)->m_VkVertexBufferMemory, 0, unSize, 0, &data);
	memcpy(data, pMesh->GetVertices(), unSize);
	vkUnmapMemory(m_VkDevice, (*ppVertexBuffer)->m_VkVertexBufferMemory);
}

void MVulkanDevice::GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap)
{
	if (*ppTextureBuffer)
		DestroyTexture(ppTextureBuffer);

	uint32_t width = pTexture->GetSize().x;
	uint32_t height = pTexture->GetSize().y;
	VkDeviceSize imageSize = width * height * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	m_ObjectDestructor.GenerateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	MByte* data;
	vkMapMemory(m_VkDevice, stagingBufferMemory, 0, imageSize, 0, (void**)&data);
	memcpy(data, pTexture->GetImageData(), static_cast<size_t>(imageSize));
	vkUnmapMemory(m_VkDevice, stagingBufferMemory);

	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = 1;
	vkSubresourceRange.layerCount = 1;

	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);
	CopyImageBuffer(stagingBuffer, textureImage, width, height, 1);
	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkSubresourceRange);

	m_ObjectDestructor.DestroyBuffer(stagingBuffer, stagingBufferMemory);

	*ppTextureBuffer = new MTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = textureImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = textureImageMemory;
	(*ppTextureBuffer)->m_VkTextureFormat = VK_FORMAT_R8G8B8A8_SRGB;
	(*ppTextureBuffer)->m_VkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	(*ppTextureBuffer)->m_VkImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void MVulkanDevice::GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap)
{
	if (*ppTextureBuffer)
		DestroyTexture(ppTextureBuffer);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	uint32_t width = vTexture[0]->GetSize().x;
	uint32_t height = vTexture[0]->GetSize().y;
	VkDeviceSize imageSize = width * height * 4;

	m_ObjectDestructor.GenerateBuffer(imageSize * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);


	MByte* data;
	vkMapMemory(m_VkDevice, stagingBufferMemory, 0, imageSize * 6, 0, (void**)&data);

	for (uint32_t i = 0; i < 6; ++i)
	{
		memcpy(data + imageSize * i, vTexture[i]->GetImageData(), static_cast<size_t>(imageSize));
	}
	vkUnmapMemory(m_VkDevice, stagingBufferMemory);


	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	CreateImageCube(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 1;
		bufferCopyRegion.imageSubresource.baseArrayLayer = face;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		offset += imageSize;
	}

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = 1;
	vkSubresourceRange.layerCount = 6;

	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);
	CopyImageBuffer(stagingBuffer, textureImage, width, height, 6);
	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkSubresourceRange);


	// Create sampler
	VkSampler vkCubeSampler = VK_NULL_HANDLE;
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler.maxAnisotropy = 1.0f;
	vkCreateSampler(m_VkDevice, &sampler, nullptr, &vkCubeSampler);


	VkImageView textureImageView;
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewInfo.subresourceRange.layerCount = 6;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.image = textureImage;
	vkCreateImageView(m_VkDevice, &viewInfo, nullptr, &textureImageView);


	vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);
	vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);


	*ppTextureBuffer = new MTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = textureImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = textureImageMemory;
	(*ppTextureBuffer)->m_VkImageView = textureImageView;
	(*ppTextureBuffer)->m_VkSampler = vkCubeSampler;
	(*ppTextureBuffer)->m_VkTextureFormat = VK_FORMAT_R8G8B8A8_SRGB;
	(*ppTextureBuffer)->m_VkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void MVulkanDevice::DestroyTexture(MTextureBuffer** ppTextureBuffer)
{
	if (*ppTextureBuffer)
	{
		m_ObjectDestructor.DestroyImageViewLater((*ppTextureBuffer)->m_VkImageView);
		m_ObjectDestructor.DestroyImageLater((*ppTextureBuffer)->m_VkTextureImage);
		m_ObjectDestructor.DestroyDeviceMemoryLater((*ppTextureBuffer)->m_VkTextureImageMemory);

		if ((*ppTextureBuffer)->m_VkSampler)
			m_ObjectDestructor.DestroySamplerLater((*ppTextureBuffer)->m_VkSampler);

		delete *ppTextureBuffer;
		*ppTextureBuffer = nullptr;
	}
}

bool MVulkanDevice::InitVulkanInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Morty App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Morty Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	
#if _DEBUG
	createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	createInfo.ppEnabledLayerNames = ValidationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if defined(MORTY_WIN)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(MORTY_ANDROID)
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(MORTY_IOS)
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		MLogManager::GetInstance()->Error(
			"Cannot find a compatible Vulkan installable client "
			"driver (ICD). Please make sure your driver supports "
			"Vulkan before continuing. The call to vkCreateInstance failed.");
		return false;
	}
	else if (result != VK_SUCCESS) {
		MLogManager::GetInstance()->Error(
			"The call to vkCreateInstance failed. Please make sure "
			"you have a Vulkan installable client driver (ICD) before "
			"continuing.");
		return false;
	}

	return true;
}

bool MVulkanDevice::InitPhysicalDevice()
{
	uint32_t nDeviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_VkInstance, &nDeviceCount, NULL);

	if (result != VK_SUCCESS || nDeviceCount < 1)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : device count < 1");
		return false;
	}

	std::vector<VkPhysicalDevice> vPhysicalDevices(nDeviceCount);
	result = vkEnumeratePhysicalDevices(m_VkInstance, &nDeviceCount, vPhysicalDevices.data());
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : vkEnumeratePhysicalDevices error.");
		return false;
	}

	m_VkPhysicalDeviceProperties = {};

	for (uint32_t i = 0; i < nDeviceCount; i++)
	{
		vkGetPhysicalDeviceProperties(vPhysicalDevices[i], &m_VkPhysicalDeviceProperties);
		MLogManager::GetInstance()->Information("API Version:    %d.%d.%d\n",
			VK_VERSION_MAJOR(m_VkPhysicalDeviceProperties.apiVersion),
			VK_VERSION_MINOR(m_VkPhysicalDeviceProperties.apiVersion),
			VK_VERSION_PATCH(m_VkPhysicalDeviceProperties.apiVersion));

		if (IsDeviceSuitable(vPhysicalDevices[i]))
		{
			m_VkPhysicalDevice = vPhysicalDevices[i];
			break;
		}
	}

	if (m_VkPhysicalDevice == VK_NULL_HANDLE)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : m_nPhysicalDeviceIndex == -1");
		return false;
	}


	return true;
}

bool MVulkanDevice::InitLogicalDevice()
{
	int nQueueFamilyIndex = FindQueueGraphicsFamilies(m_VkPhysicalDevice);

	float priorities[] = { 1.0f };	//range 0~1
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = NULL;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = nQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priorities[0];


	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(m_VkPhysicalDevice, &deviceFeatures);


	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = DeviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;

	//����Ĭ���˵�ǰ�����壨nQueueFamilyIndex����֧���������͵Ĺ���
	//���ܻ���ڲ�֧������ǰ��Ļ�ϻ��Ƶģ����������Ҫ�ڴ���logicalDevice֮ǰ�������ڵ�surface��Ȼ��check֧��֧�֡�
	//���̫�����ˣ��Ȳ�����
	VkResult result = vkCreateDevice(m_VkPhysicalDevice, &deviceInfo, NULL, &m_VkDevice);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : vkCreateDevice error.");
		return false;
	}

	vkGetDeviceQueue(m_VkDevice, nQueueFamilyIndex, 0, &m_VkGraphicsQueue);

	return true;
}

bool MVulkanDevice::InitCommandPool()
{
	uint32_t unQueueFamilyIndex = FindQueueGraphicsFamilies(m_VkPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = unQueueFamilyIndex;
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS)
		return false;


	return true;
}

bool MVulkanDevice::InitDefaultTexture()
{
	m_WhiteTexture.SetSize(Vector2(4, 4));
	m_WhiteTexture.FillColor(MColor(1, 1, 1, 1));
	m_WhiteTexture.GenerateBuffer(this, false);

	return true;
}

bool MVulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
{
	if (-1 == FindQueueGraphicsFamilies(device))
		return false;

	if (!CheckDeviceExtensionSupport(device))
		return false;

	return true;
}

int MVulkanDevice::FindQueueGraphicsFamilies(VkPhysicalDevice device)
{
	int graphicsFamily = -1;

	uint32_t unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, vQueueProperties.data());

	for (int i = 0; i < unQueueFamilyCount; ++i)
	{
		if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			graphicsFamily = i;

		if (graphicsFamily >= 0)
			break;
	}

	return graphicsFamily;
}

int MVulkanDevice::FindQueuePresentFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	int nPresentFamily = -1;

	uint32_t unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &unQueueFamilyCount, NULL);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &unQueueFamilyCount, vQueueProperties.data());

	VkBool32 bSupportsPresenting(VK_FALSE);
	for (uint32_t i = 0; i < unQueueFamilyCount; ++i)
	{
		GetPhysicalDeviceSurfaceSupportKHR(m_VkPhysicalDevice, i, surface, &bSupportsPresenting);

		if (vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (bSupportsPresenting == VK_TRUE) {
				nPresentFamily = i;
				break;
			}
		}
	}

	return nPresentFamily;
}

bool MVulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void MVulkanDevice::GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const uint32_t& unWidth, const uint32_t& unHeight)
{
	if (*ppTextureBuffer)
		DestroyDepthTexture(ppTextureBuffer);

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	int depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	CreateImage(unWidth, unHeight, m_VkDepthTextureFormat, VK_IMAGE_TILING_OPTIMAL, depthUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = CreateImageView(depthImage, m_VkDepthTextureFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	VkFilter vkShadowMapFilter = FormatIsFilterable(m_VkDepthTextureFormat, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;


	VkSampler depthSampler;
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = vkShadowMapFilter;
	sampler.minFilter = vkShadowMapFilter;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	vkCreateSampler(m_VkDevice, &sampler, nullptr, &depthSampler);


	*ppTextureBuffer = new MDepthTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = depthImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = depthImageMemory;
	(*ppTextureBuffer)->m_VkTextureFormat = m_VkDepthTextureFormat;
	(*ppTextureBuffer)->m_VkImageView = depthImageView;
	(*ppTextureBuffer)->m_VkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	(*ppTextureBuffer)->m_VkSampler = depthSampler;
}

void MVulkanDevice::DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer)
{
	if (*ppTextureBuffer)
	{
		m_ObjectDestructor.DestroyImageViewLater((*ppTextureBuffer)->m_VkImageView);
		m_ObjectDestructor.DestroyImageLater((*ppTextureBuffer)->m_VkTextureImage);
		m_ObjectDestructor.DestroyDeviceMemoryLater((*ppTextureBuffer)->m_VkTextureImageMemory);
		m_ObjectDestructor.DestroySamplerLater((*ppTextureBuffer)->m_VkSampler);

		delete* ppTextureBuffer;
		*ppTextureBuffer = nullptr;
	}
}

bool MVulkanDevice::CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro)
{
	if (*ppShaderBuffer)
	{
		CleanShader(ppShaderBuffer);
	}

	std::vector<uint32_t> spirv;
	m_ShaderCompiler.CompileShader(strShaderPath, eShaderType, macro, spirv);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return false;

	spirv_cross::Compiler compiler(spirv);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.stage = eShaderType == MShader::MEShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = eShaderType == MShader::MEShaderType::Vertex ? "VS" : "PS";
	

	shaderStageInfo.pSpecializationInfo = nullptr;

	MShaderBuffer* pBuffer = nullptr;
	if (MShader::MEShaderType::Vertex == eShaderType)
	{
		MVertexShaderBuffer* pVertexBuffer = new MVertexShaderBuffer();
		m_ShaderCompiler.GetVertexInputState(compiler, pVertexBuffer);
		pBuffer = pVertexBuffer;
	}
	else if (MShader::MEShaderType::Pixel == eShaderType)
	{
		MPixelShaderBuffer* pPixelBuffer = new MPixelShaderBuffer();
		pBuffer = pPixelBuffer;
	}

	pBuffer->m_VkShaderModule = shaderModule;
	pBuffer->m_VkShaderStageInfo = shaderStageInfo;
	m_ShaderCompiler.GetShaderParam(compiler, pBuffer);



	*ppShaderBuffer = pBuffer;
	return true;
}

void MVulkanDevice::CleanShader(MShaderBuffer** ppShaderBuffer)
{
	if (nullptr == *ppShaderBuffer)
		return;

	MShaderBuffer* pBuffer = (*ppShaderBuffer);

// 	for (MShaderParam* param : pBuffer->m_vShaderParamsTemplate)
// 	{
// 		m_ObjectDestructor.DestroyBufferLater(param->m_VkBuffer);
// 		m_ObjectDestructor.DestroyDeviceMemoryLater(param->m_VkBufferMemory);
// 
// 	}

	m_ObjectDestructor.DestroyShaderModuleLater(pBuffer->m_VkShaderModule);

	delete* ppShaderBuffer;
	*ppShaderBuffer = nullptr;
}

bool MVulkanDevice::GenerateShaderParamSet(MShaderParamSet* pParamSet)
{
	if (!pParamSet)
		return false;

	//DestroyShaderParamSet(pParamSet);

	
	m_PipelineManager.GenerateShaderParamSet(pParamSet);

	return true;
}

void MVulkanDevice::DestroyShaderParamSet(MShaderParamSet* pParamSet)
{
	m_PipelineManager.DestroyShaderParamSet(pParamSet);
}

bool MVulkanDevice::GenerateSubpassTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const METextureLayout& eType, const uint32_t& unWidth, const uint32_t& unHeight)
{
	if (*ppTextureBuffer)
		DestroyRenderTextureBuffer(ppTextureBuffer);


	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	VkFormat textureFormat = GetFormat(eType);

	CreateImage(unWidth, unHeight, textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = 1;
	vkSubresourceRange.layerCount = 1;

	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vkSubresourceRange);

	(*ppTextureBuffer) = new MRenderTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = textureImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = textureImageMemory;
	(*ppTextureBuffer)->m_VkTextureFormat = textureFormat;
	(*ppTextureBuffer)->m_VkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	GenerateRenderTargetView(*ppTextureBuffer);

	return true;
}

bool MVulkanDevice::GenerateRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const METextureLayout& eType, const uint32_t& unWidth, const unsigned& unHeight)
{
	if (*ppTextureBuffer)
		DestroyRenderTextureBuffer(ppTextureBuffer);


	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	VkFormat textureFormat = GetFormat(eType);

	//TODO VK_IMAGE_USAGE_TRANSFER_SRC_BIT is not necessary.
	CreateImage(unWidth, unHeight, textureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = 1;
	vkSubresourceRange.layerCount = 1;

	TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkSubresourceRange);

	(*ppTextureBuffer) = new MRenderTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = textureImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = textureImageMemory;
	(*ppTextureBuffer)->m_VkTextureFormat = textureFormat;
	(*ppTextureBuffer)->m_VkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	GenerateRenderTargetView(*ppTextureBuffer);
	
	return true;
}

void MVulkanDevice::DestroyRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer)
{
	if (*ppTextureBuffer)
	{
		DestroyRenderTargetView(*ppTextureBuffer);
		m_ObjectDestructor.DestroyImageLater((*ppTextureBuffer)->m_VkTextureImage);
		m_ObjectDestructor.DestroyDeviceMemoryLater((*ppTextureBuffer)->m_VkTextureImageMemory);

		delete (*ppTextureBuffer);
		*ppTextureBuffer = nullptr;
	}
}

bool MVulkanDevice::GenerateRenderTarget(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget)
{
	if (!pRenderTarget)
	{
		MLogManager::GetInstance()->Error("MVulkanDevice::GenerateRenderTarget error, rt == nullptr");
		return false;
	}

	if (!pRenderPass)
	{
		MLogManager::GetInstance()->Error("MVulkanDevice::GenerateRenderTarget error, rp == nullptr");
		return false;
	}

	VkRenderPass vkRenderPass = pRenderPass->m_aVkRenderPass[0];

	if (VK_NULL_HANDLE == vkRenderPass)
	{
		MLogManager::GetInstance()->Error("MVulkanDevice::GenerateRenderTarget error, vkrp == nullptr");
		return false;
	}

	Vector2 v2Size = pRenderTarget->GetSize();

	if (v2Size.x < 1)
		v2Size.x = 1;
	if (v2Size.y < 1)
		v2Size.y = 1;

	if(!dynamic_cast<MVulkanRenderTarget*>(pRenderTarget))
	{
		pRenderTarget->m_VkExtend.width = v2Size.x;
		pRenderTarget->m_VkExtend.height = v2Size.y;
	}

	for (uint32_t i = 0; i < pRenderTarget->GetMFrameBufferNum(); ++i)
	{
		MFrameBuffer* pFrameBuffer = pRenderTarget->GetFrameBuffer(i);

		std::vector<VkImageView> vAttachments;

		for (MIRenderBackTexture* pBackTexture : pFrameBuffer->vBackTextures)
		{
			if (pBackTexture)
			{
				vAttachments.push_back(pBackTexture->GetRenderBuffer()->m_VkImageView);
			}
		}

		if (pFrameBuffer->pDepthTexture)
		{
			MDepthTextureBuffer* pDepthBuffer = pFrameBuffer->pDepthTexture->GetDepthBuffer();
			vAttachments.push_back(pDepthBuffer->m_VkImageView);
		}

		

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		//This renderpass only used to match format.
		framebufferInfo.renderPass = vkRenderPass;

		pFrameBuffer->vkExtend = pRenderTarget->m_VkExtend;

		framebufferInfo.attachmentCount = vAttachments.size();
		framebufferInfo.pAttachments = vAttachments.data();
		framebufferInfo.width = v2Size.x;
		framebufferInfo.height = v2Size.y;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &pFrameBuffer->vkFrameBuffer);
	}

	
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (VkSemaphore& vkSemaphore : pRenderTarget->m_aVkRenderFinishedSemaphore)
	{
		if (vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &vkSemaphore) != VK_SUCCESS)
			return false;
	}

	return true;
}

void MVulkanDevice::DestroyRenderTarget(MIRenderTarget* pRenderTarget)
{
	if (!pRenderTarget)
		return;

	MIRenderTarget* pVkRenderTarget = pRenderTarget;

	for (uint32_t i = 0; i < pRenderTarget->GetMFrameBufferNum(); ++i)
	{
		MFrameBuffer* pFrameBuffer = pVkRenderTarget->GetFrameBuffer(i);

		if (pFrameBuffer->vkFrameBuffer)
		{
			m_ObjectDestructor.DestroyFramebufferLater(pFrameBuffer->vkFrameBuffer);
			pFrameBuffer->vkFrameBuffer = VK_NULL_HANDLE;
		}
	}

	for (uint32_t i = 0; i < pRenderTarget->m_VkCommandBuffers.size(); ++i)
	{
		m_ObjectDestructor.DestroyCommandBufferLater(pRenderTarget->m_VkCommandBuffers[i]);
		pRenderTarget->m_VkCommandBuffers[i] = VK_NULL_HANDLE;
	}

	for (uint32_t i = 0; i < pRenderTarget->m_aVkRenderFinishedSemaphore.size(); ++i)
	{
		m_ObjectDestructor.DestroySemaphoreLater(pRenderTarget->m_aVkRenderFinishedSemaphore[i]);
		pRenderTarget->m_aVkRenderFinishedSemaphore[i] = VK_NULL_HANDLE;
	}


}

bool MVulkanDevice::GenerateShaderParamBuffer(MShaderConstantParam* pParam)
{
	if (VK_NULL_HANDLE != pParam->m_VkBuffer[0])
		DestroyShaderParamBuffer(pParam);

	if (pParam)
	{
		return m_DynamicUniformBufferPool.AllowBufferMemory(pParam);
	}

	return false;
}

void MVulkanDevice::DestroyShaderParamBuffer(MShaderConstantParam* pParam)
{
	if (pParam)
	{
		m_DynamicUniformBufferPool.FreeBufferMemory(pParam);
	}
}
bool MVulkanDevice::GenerateRenderPass(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget)
{	
	if (!pRenderTarget)
		return false;

	if (!pRenderPass)
		return false;

	MFrameBuffer* pFrameBuffer = pRenderTarget->GetCurrFrameBuffer();
	if (!pFrameBuffer)
		return false;

	for (uint32_t i = 0; i < pRenderPass->m_aVkRenderPass.size(); ++i)
	{
		if (VK_NULL_HANDLE != pRenderPass->m_aVkRenderPass[i])
		{
			DestroyRenderPass(pRenderPass);
			break;
		}
	}

	VkRenderPass renderPass;

	uint32_t unBackNum = pRenderPass->m_vBackDesc.size();

	std::vector<VkAttachmentDescription> vAttachmentDesc;


	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MIRenderBackTexture* pBackTexture = pFrameBuffer->vBackTextures[i];
		if (!pBackTexture)
		{
			MLogManager::GetInstance()->Error("MVulkanDevice::GenerateRenderPass error: bt == nullptr");
			return false;
		}
		
		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();


		if (pRenderPass->m_vBackDesc[i].bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		if (MTextureBuffer* pTexBuffer = pBackTexture->GetBuffer())
			colorAttachment.format = pTexBuffer->m_VkTextureFormat;
		else
			colorAttachment.format = GetFormat(pBackTexture->GetType());

		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (!pBackTexture->GetRenderBuffer())
		{
			pBackTexture->SetSize(pRenderTarget->GetSize());
			pBackTexture->GenerateBuffer(this);
		}

		colorAttachment.initialLayout =	pBackTexture->GetBuffer()->m_VkImageLayout;

		if (dynamic_cast<MVulkanRenderTarget*>(pRenderTarget))
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		else if (pBackTexture->GetBuffer())
			colorAttachment.finalLayout = pBackTexture->GetBuffer()->m_VkImageLayout;
		else
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	if (pRenderTarget->GetDepthEnable())
	{
		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();

		if (pRenderPass->m_DepthDesc.bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		colorAttachment.format = m_VkDepthTextureFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;			//for Shader
	}


	std::vector<VkSubpassDescription> vSubpass;

	std::vector<std::vector<VkAttachmentReference>> vOutAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vOutDepthAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vInAttachmentRef;
	std::vector<std::vector<uint32_t>> vUnusedAttachmentRef;

	// default subpass
	if (pRenderPass->m_vSubpass.empty())
	{
		vOutAttachmentRef.resize(1);
		vOutDepthAttachmentRef.resize(1);

		vSubpass.push_back(VkSubpassDescription());
		VkSubpassDescription& vkSubpass = vSubpass.back();
		vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		
		for (uint32_t i = 0; i < unBackNum; ++i)
			vOutAttachmentRef[0].push_back({ uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		vkSubpass.colorAttachmentCount = unBackNum;

		if (pRenderTarget->GetDepthEnable())
		{
			vOutDepthAttachmentRef[0] = { { uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };
			vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[0].data();
		}

		vkSubpass.pColorAttachments = vOutAttachmentRef[0].data();
	}
	else  //
	{
		uint32_t unSubpassNum = pRenderPass->m_vSubpass.size();
		vOutAttachmentRef.resize(unSubpassNum);
		vOutDepthAttachmentRef.resize(unSubpassNum);
		vInAttachmentRef.resize(unSubpassNum);
		vUnusedAttachmentRef.resize(unSubpassNum);

		for (uint32_t nSubpassIdx = 0; nSubpassIdx < unSubpassNum; ++nSubpassIdx)
		{
			MSubpass& subpass = pRenderPass->m_vSubpass[nSubpassIdx];

			vSubpass.push_back(VkSubpassDescription());
			VkSubpassDescription& vkSubpass = vSubpass.back();
			vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			std::set<uint32_t> vUsedAttachIndex;

			for (uint32_t i = 0; i < subpass.m_vOutputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vOutputIndex[i];

				vOutAttachmentRef[nSubpassIdx].push_back({});
				VkAttachmentReference& vkAttachRef = vOutAttachmentRef[nSubpassIdx].back();

				vkAttachRef.attachment = nBackIdx;
				vkAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				vUsedAttachIndex.insert(nBackIdx);
			}
			
			if (pRenderTarget->GetDepthEnable())
			{
				vOutDepthAttachmentRef[nSubpassIdx] = { { uint32_t(vOutAttachmentRef[nSubpassIdx].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };
				vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[nSubpassIdx].data();
			}

			vkSubpass.colorAttachmentCount = vOutAttachmentRef[nSubpassIdx].size();
			vkSubpass.pColorAttachments = vOutAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < subpass.m_vInputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vInputIndex[i];

				vInAttachmentRef[nSubpassIdx].push_back({});
				VkAttachmentReference& vkAttachRef = vInAttachmentRef[nSubpassIdx].back();

				vkAttachRef.attachment = nBackIdx;
				vkAttachRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vUsedAttachIndex.insert(nBackIdx);
			}

			vkSubpass.inputAttachmentCount = vInAttachmentRef[nSubpassIdx].size();
			vkSubpass.pInputAttachments = vInAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < unBackNum; ++i)
			{
				if (vUsedAttachIndex.find(i) == vUsedAttachIndex.end())
				{
					vUnusedAttachmentRef[nSubpassIdx].push_back(i);
				}
			}

			vkSubpass.preserveAttachmentCount = vUnusedAttachmentRef[nSubpassIdx].size();
			vkSubpass.pPreserveAttachments = vUnusedAttachmentRef[nSubpassIdx].data();

		}
	}

	std::vector<VkSubpassDependency> vSubpassDependencies;
	for (uint32_t nSubpassIdx = 1; nSubpassIdx < vSubpass.size(); ++nSubpassIdx)
	{
		for (int nDependantIdx = 0; nDependantIdx < nSubpassIdx; ++nDependantIdx)
		{
			vSubpassDependencies.push_back({});
			VkSubpassDependency& depend = vSubpassDependencies.back();
			depend.srcSubpass = nDependantIdx;
			depend.dstSubpass = nSubpassIdx;
//          depend.dstSubpass = (subpass<subpassCount) ? subpass : VK_SUBPASS_EXTERNAL;
			depend.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dependencyFlags = 0;
		}
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = vAttachmentDesc.size();
	renderPassInfo.pAttachments = vAttachmentDesc.data();
	renderPassInfo.subpassCount = vSubpass.size();
	renderPassInfo.pSubpasses = vSubpass.data();
	renderPassInfo.dependencyCount = vSubpassDependencies.size();
	renderPassInfo.pDependencies = vSubpassDependencies.data();


	for (uint32_t i = 0; i < pRenderPass->m_aVkRenderPass.size(); ++i)
	{
		if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			return false;
		}

		pRenderPass->m_aVkRenderPass[i] = renderPass;
	}

	m_PipelineManager.RegisterRenderPass(pRenderPass);
	return true;
}

void MVulkanDevice::DestroyRenderPass(MRenderPass* pRenderPass)
{
	if (pRenderPass)
	{
		for (uint32_t i = 0; i < pRenderPass->m_aVkRenderPass.size(); ++i)
		{
			if (VK_NULL_HANDLE != pRenderPass->m_aVkRenderPass[i])
			{
				m_ObjectDestructor.DestroyRenderPassLater(pRenderPass->m_aVkRenderPass[i]);
				pRenderPass->m_aVkRenderPass[i] = VK_NULL_HANDLE;
			}
		}

		m_PipelineManager.UnRegisterRenderPass(pRenderPass);
	}
}

bool MVulkanDevice::GenerateRenderTargetView(MRenderTextureBuffer* pTextureBuffer)
{
	return pTextureBuffer->m_VkImageView = CreateImageView(pTextureBuffer->m_VkTextureImage, pTextureBuffer->m_VkTextureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void MVulkanDevice::DestroyRenderTargetView(MRenderTextureBuffer* pTextureBuffer)
{
	if (pTextureBuffer->m_VkImageView)
	{
		m_ObjectDestructor.DestroyImageViewLater(pTextureBuffer->m_VkImageView);
	}
}

bool MVulkanDevice::RegisterMaterial(MMaterial* pMaterial)
{
	return m_PipelineManager.RegisterMaterial(pMaterial);
}

bool MVulkanDevice::UnRegisterMaterial(MMaterial* pMaterial)
{
	return m_PipelineManager.UnRegisterMaterial(pMaterial);
}

#endif