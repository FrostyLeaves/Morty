#include "MVulkanDevice.h"
#include "MMesh.h"
#include "MShader.h"
#include "MTexture.h"
#include "MResource.h"
#include "MFileHelper.h"
#include "MRenderStructure.h"

#include <set>
#include <array>

#if RENDER_GRAPHICS == MORTY_VULKAN


#include "MVulkanRenderTarget.h"

#ifdef MORTY_WIN
#include "vulkan/vulkan_win32.h"
#endif


const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
	, m_VkDevice(VK_NULL_HANDLE)
	, m_VkGraphicsQueue(VK_NULL_HANDLE)
{

}

MVulkanDevice::~MVulkanDevice()
{

}

bool MVulkanDevice::Initialize()
{
	if (!InitVulkanInstance())
		return false;

	if (!InitPhysicalDevice())
		return false;

	if (!InitLogicalDevice())
		return false;

	if (!InitCommandPool())
		return false;
	
	if (!InitDescriptorPool())
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

	return -1;
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
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
		vkMapMemory(m_VkDevice, vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, vertexBufferMemory);

		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);
		vkMapMemory(m_VkDevice, indexBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, indexBufferMemory);
	}
	else
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);
		vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);


		VkBuffer stagingIdxBuffer;
		VkDeviceMemory stagingIdxBufferMemory;
		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIdxBuffer, stagingIdxBufferMemory);

		vkMapMemory(m_VkDevice, stagingIdxBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, stagingIdxBufferMemory);

		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingIdxBuffer, indexBuffer, indexBufferSize);

		vkDestroyBuffer(m_VkDevice, stagingIdxBuffer, nullptr);
		vkFreeMemory(m_VkDevice, stagingIdxBufferMemory, nullptr);
	}

	//TODO 优化内存分配机制，以合理利用内存。比如建立可复用的内存池，内存管理器



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

void MVulkanDevice::CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height)
{
	VkCommandBuffer commandBuffer = BeginCommands();


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
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
		DestroyBuffer((*ppVertexBuffer)->m_VkVertexBuffer, (*ppVertexBuffer)->m_VkVertexBufferMemory);
		DestroyBuffer((*ppVertexBuffer)->m_VkIndexBuffer, (*ppVertexBuffer)->m_VkIndexBufferMemory);
		delete *ppVertexBuffer;
		*ppVertexBuffer = nullptr;
	}
}

void MVulkanDevice::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = BeginCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndCommands(commandBuffer);
}

VkImageView MVulkanDevice::CreateImageView(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
	vkQueueWaitIdle(m_VkGraphicsQueue);

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

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	Vector2 v2Size = pTexture->GetSize();
	VkDeviceSize imageSize = v2Size.x * v2Size.y * 4;

	
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(v2Size.x);
	imageInfo.extent.height = static_cast<uint32_t>(v2Size.y);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
	{
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
	{
		vkDestroyImage(m_VkDevice, textureImage, nullptr);
		return;
	}

	vkBindImageMemory(m_VkDevice, textureImage, textureImageMemory, 0);



	GenerateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(m_VkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pTexture->GetImageData(), static_cast<size_t>(imageSize));
	vkUnmapMemory(m_VkDevice, stagingBufferMemory);

	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyImageBuffer(stagingBuffer, textureImage, static_cast<uint32_t>(v2Size.x), static_cast<uint32_t>(v2Size.y));
	TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(m_VkDevice, stagingBuffer, nullptr);
	vkFreeMemory(m_VkDevice, stagingBufferMemory, nullptr);


	*ppTextureBuffer = new MTextureBuffer();
	(*ppTextureBuffer)->m_VkTextureImage = textureImage;
	(*ppTextureBuffer)->m_VkTextureImageMemory = textureImageMemory;
	(*ppTextureBuffer)->m_VkImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void MVulkanDevice::DestroyTexture(MTextureBuffer** ppTextureBuffer)
{
	if (*ppTextureBuffer)
	{
		vkDestroyImageView(m_VkDevice, (*ppTextureBuffer)->m_VkImageView, nullptr);
		vkDestroyImage(m_VkDevice, (*ppTextureBuffer)->m_VkTextureImage, nullptr);
		vkFreeMemory(m_VkDevice, (*ppTextureBuffer)->m_VkTextureImageMemory, nullptr);

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

	VkPhysicalDeviceProperties physicalProperties = {};

	for (uint32_t i = 0; i < nDeviceCount; i++)
	{
		// 		vkGetPhysicalDeviceProperties(m_vPhysicalDevices[i], &physicalProperties);
		// 		MLogManager::GetInstance()->Information("API Version:    %d.%d.%d\n",
		// 			VK_VERSION_MAJOR(physicalProperties.apiVersion),
		// 			VK_VERSION_MINOR(physicalProperties.apiVersion),
		// 			VK_VERSION_PATCH(physicalProperties.apiVersion));

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

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = DeviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;

	//这里默认了当前队列族（nQueueFamilyIndex）都支持所有类型的功能
	//可能会存在不支持往当前屏幕上绘制的，处理这个需要在创建logicalDevice之前创建窗口的surface，然后check支不支持。
	//这个太恶心了，先不处理
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

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkCommandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &m_VkCommandBuffer);

	return true;
}

bool MVulkanDevice::InitDescriptorPool()
{
	uint32_t unSwapChainNum = 1;

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = unSwapChainNum;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	poolInfo.maxSets = unSwapChainNum;

	if (vkCreateDescriptorPool(m_VkDevice, &poolInfo, nullptr, &m_VkDescriptorPool) != VK_SUCCESS)
	{
		return false;
	}

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

bool MVulkanDevice::GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_VkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VkDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (-1 == allocInfo.memoryTypeIndex)
	{
		vkDestroyBuffer(m_VkDevice, buffer, nullptr);
		return false;
	}

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		vkDestroyBuffer(m_VkDevice, buffer, nullptr);
		return false;
	}

	vkBindBufferMemory(m_VkDevice, buffer, bufferMemory, 0);

	return true;
}

void MVulkanDevice::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	vkDestroyBuffer(m_VkDevice, buffer, nullptr);
	vkFreeMemory(m_VkDevice, bufferMemory, nullptr);
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
	createInfo.codeSize = spirv.size();
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
	
	//TODO 用来定义类似于hlsl中的宏的变量
	shaderStageInfo.pSpecializationInfo = nullptr;

	MShaderBuffer* pBuffer = nullptr;
	if (MShader::MEShaderType::Vertex == eShaderType)
	{
		MVertexShaderBuffer* pVertexBuffer = new MVertexShaderBuffer();
		m_ShaderCompiler.GetVertexInputState(compiler, pVertexBuffer->m_VkVertexInputStateInfo);
		pBuffer = pVertexBuffer;
	}
	else if (MShader::MEShaderType::Pixel == eShaderType)
	{
		MPixelShaderBuffer* pPixelBuffer = new MPixelShaderBuffer();
		pBuffer = pPixelBuffer;
	}

	pBuffer->m_VkShaderStageInfo = shaderStageInfo;
	m_ShaderCompiler.GetShaderParam(compiler, pBuffer);



	*ppShaderBuffer = pBuffer;
	return true;
}

void MVulkanDevice::CleanShader(MShaderBuffer** ppShaderBuffer)
{
	if (nullptr == *ppShaderBuffer)
		return;

	if (MVertexShaderBuffer* pBuffer = dynamic_cast<MVertexShaderBuffer*>(*ppShaderBuffer))
	{

	}
	else if (MPixelShaderBuffer* pBuffer = dynamic_cast<MPixelShaderBuffer*>(*ppShaderBuffer))
	{

	}

	delete* ppShaderBuffer;
	*ppShaderBuffer = nullptr;
}

bool MVulkanDevice::GenerateRenderTarget(MIRenderTarget* pRenderTarget, uint32_t nWidth, uint32_t nHeight)
{
	MVulkanRenderTarget* pVkRenderTarget = dynamic_cast<MVulkanRenderTarget*>(pRenderTarget);
	if (nullptr == pVkRenderTarget)
		return false;

	if (nWidth < 1)
		nWidth = 1;
	if (nHeight < 1)
		nHeight = 1;

	GenerateRenderPass(pVkRenderTarget, &pVkRenderTarget->m_VkRenderPass);
	if (VK_NULL_HANDLE == pVkRenderTarget->m_VkRenderPass)
		return false;

	VkImageViewCreateInfo createInfo = {};

	pVkRenderTarget->m_vSwapchainImageView.resize(pVkRenderTarget->m_vSwapchainImages.size());
	pVkRenderTarget->swapChainFramebuffers.resize(pVkRenderTarget->m_vSwapchainImages.size());
	for (uint32_t i = 0; i < pVkRenderTarget->m_vSwapchainImages.size(); ++i)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = pVkRenderTarget->m_vSwapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = pVkRenderTarget->m_VkColorFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(m_VkDevice, &createInfo, nullptr, &pVkRenderTarget->m_vSwapchainImageView[i]);

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = pRenderTarget->m_VkRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &pVkRenderTarget->m_vSwapchainImageView[i];
		framebufferInfo.width = nWidth;
		framebufferInfo.height = nHeight;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &pVkRenderTarget->swapChainFramebuffers[i]);

		//TODO 不优雅
		MRenderTargetView target;
		target.m_VkRenderTextureImage = pVkRenderTarget->m_vSwapchainImages[i];
		target.m_VkFrameBuffer = pVkRenderTarget->swapChainFramebuffers[i];
		pVkRenderTarget->m_RenderTargetView.push_back(target);
	}

	

	//TODO Multiple RenderTarget


	return true;
}



bool MVulkanDevice::GenerateShaderParamBuffer(MShaderParam* pParam)
{
	if (VK_NULL_HANDLE != pParam->m_VkBuffer)
		DestroyShaderParamBuffer(pParam);

	if (pParam)
	{
		//Memory
		return GenerateBuffer(pParam->var.GetSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			pParam->m_VkBuffer, pParam->m_VkBufferMemory);
		

		VkWriteDescriptorSet writeDescriptorSet;

		writeDescriptorSet.dstSet = VkDescriptorSet();

		return true;
	}

	return false;
}

void MVulkanDevice::DestroyShaderParamBuffer(MShaderParam* pParam)
{
	if (pParam)
	{
		DestroyBuffer(pParam->m_VkBuffer, pParam->m_VkBufferMemory);
	}
}

bool MVulkanDevice::GenerateRenderPass(MIRenderTarget* pRenderTarget, MRenderPass* pRenderPass)
{
	VkRenderPass renderPass;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = pRenderTarget->m_VkColorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		return false;
	}

	pRenderPass->m_VkRenderPass = renderPass;

	return true;
}

void MVulkanDevice::DestroyRenderPass(MRenderPass* pRenderPass)
{

}

#endif