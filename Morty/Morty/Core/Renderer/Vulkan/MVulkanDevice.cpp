#include "MVulkanDevice.h"
#include "MShader.h"
#include "MFileHelper.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "spirv_cross/spirv_cross.hpp"

#include "MVulkanRenderTarget.h"

#include <set>

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
	, m_VKInstance(VK_NULL_HANDLE)
	, m_VKPhysicalDevice(VK_NULL_HANDLE)
	, m_VKDevice(VK_NULL_HANDLE)
	, m_VKGraphicsQueue(VK_NULL_HANDLE)
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
	


	GET_INSTANCE_PROC_ADDR(m_VKInstance, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(m_VKInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_VKInstance, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(m_VKInstance, GetPhysicalDeviceSurfacePresentModesKHR);

	GET_DEVICE_PROC_ADDR(m_VKDevice, CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_VKDevice, DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(m_VKDevice, GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(m_VKDevice, AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(m_VKDevice, QueuePresentKHR);


	return true;
}

void MVulkanDevice::Release()
{
	vkDestroyInstance(m_VKInstance, NULL);
}

void MVulkanDevice::GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable /*= false*/)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
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

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VKInstance);

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
	unsigned int nDeviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_VKInstance, &nDeviceCount, NULL);

	if (result != VK_SUCCESS || nDeviceCount < 1)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : device count < 1");
		return false;
	}

	std::vector<VkPhysicalDevice> vPhysicalDevices(nDeviceCount);
	result = vkEnumeratePhysicalDevices(m_VKInstance, &nDeviceCount, vPhysicalDevices.data());
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
			m_VKPhysicalDevice = vPhysicalDevices[i];
			break;
		}
	}

	if (m_VKPhysicalDevice == VK_NULL_HANDLE)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : m_nPhysicalDeviceIndex == -1");
		return false;
	}


	return true;
}

bool MVulkanDevice::InitLogicalDevice()
{
	int nQueueFamilyIndex = FindQueueGraphicsFamilies(m_VKPhysicalDevice);

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
	VkResult result = vkCreateDevice(m_VKPhysicalDevice, &deviceInfo, NULL, &m_VKDevice);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Initialize Vulkan Error : vkCreateDevice error.");
		return false;
	}

	vkGetDeviceQueue(m_VKDevice, nQueueFamilyIndex, 0, &m_VKGraphicsQueue);

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

	unsigned int unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_VKPhysicalDevice, &unQueueFamilyCount, NULL);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_VKPhysicalDevice, &unQueueFamilyCount, vQueueProperties.data());

	VkBool32 bSupportsPresenting(VK_FALSE);
	for (unsigned int i = 0; i < unQueueFamilyCount; ++i)
	{
		GetPhysicalDeviceSurfaceSupportKHR(m_VKPhysicalDevice, i, surface, &bSupportsPresenting);

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

bool MVulkanDevice::CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType, const MShaderMacro& macro)
{
	MString code;
	if (!MFileHelper::ReadString(strShaderPath, code))
		return false;

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_VKDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return false;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = eShaderType == MShader::MEShaderType::Vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
	vertShaderStageInfo.module = shaderModule;
	vertShaderStageInfo.pName = eShaderType == MShader::MEShaderType::Vertex ? "VS" : "PS";
	
	//TODO 用来定义类似于hlsl中的宏的变量
	vertShaderStageInfo.pSpecializationInfo = nullptr;



	return true;
}

bool MVulkanDevice::GenerateRenderTarget(MIRenderTarget* pRenderTarget, unsigned int nWidth, unsigned int nHeight)
{
	MVulkanRenderTarget* pVkRenderTarget = dynamic_cast<MVulkanRenderTarget*>(pRenderTarget);
	if (nullptr == pVkRenderTarget)
		return false;

	if (nWidth < 1)
		nWidth = 1;
	if (nHeight < 1)
		nHeight = 1;

	VkImageViewCreateInfo createInfo = {};

	for (unsigned int i = 0; i < pVkRenderTarget->m_vSwapchainImages.size(); ++i)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = pVkRenderTarget->m_vSwapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = pVkRenderTarget->m_VKColorFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(m_VKDevice, &createInfo, nullptr, &pVkRenderTarget->m_vSwapchainImageView[i]);

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &pVkRenderTarget->m_vSwapchainImageView[i];
		framebufferInfo.width = nWidth;
		framebufferInfo.height = nHeight;
		framebufferInfo.layers = 1;
		vkCreateFramebuffer(m_VKDevice, &framebufferInfo, nullptr, &pVkRenderTarget->swapChainFramebuffers[i]);
	}

	
	unsigned int unTargetViewSize = pRenderTarget->GetTargetViewNum();

	//TODO Multiple RenderTarget


	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = pVkRenderTarget->m_VKColorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//VK_ATTACHMENT_LOAD_OP_LOAD：保留附件的现有内容
	//VK_ATTACHMENT_LOAD_OP_CLEAR：在开始时将值清除为常数
	//VK_ATTACHMENT_LOAD_OP_DONT_CARE：现有内容未定义；我们不在乎他们
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//VK_ATTACHMENT_STORE_OP_STORE：渲染的内容将存储在内存中，以后可以读取
	//VK_ATTACHMENT_STORE_OP_DONT_CARE：渲染操作后，帧缓冲区的内容将不确定
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;


	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	//交换链




	return true;
}



#endif