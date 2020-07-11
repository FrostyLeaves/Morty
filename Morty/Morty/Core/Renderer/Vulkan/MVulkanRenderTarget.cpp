#include "MVulkanRenderTarget.h"

#include "MTexture.h"
#include "MViewport.h"
#include "MVulkanRenderer.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#ifdef MORTY_WIN
#include "vulkan/vulkan_win32.h"
#endif

MVulkanRenderTarget::MVulkanRenderTarget(MVulkanDevice* pDevice)
	:MIRenderTarget()
	, m_pDevice(pDevice)
	, m_pDepthTexture(new MRenderDepthTexture())
	, m_pView(nullptr)
{

}

MVulkanRenderTarget::~MVulkanRenderTarget()
{

}

MRenderTextureBuffer* MVulkanRenderTarget::GetBackBuffer(const uint32_t& unIndex)
{
	return m_vBackBuffers[unIndex];
}

MColor MVulkanRenderTarget::GetBackClearColor(const uint32_t& unIndex)
{
	return m_pView->GetBackColor();
}

void MVulkanRenderTarget::Render(MIRenderer* pRenderer)
{
	MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(pRenderer);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_pDevice->m_VkDevice, m_VkSwapchain, UINT64_MAX, pVkRenderer->m_VkImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	//TODO 10
	if (imageIndex < 10)
	{
		pVkRenderer->SetFrameIndex(imageIndex);
		pRenderer->Render(this);
	}
}

void MVulkanRenderTarget::OnRender(MIRenderer* pRenderer)
{

	m_pView->OnRenderBegin();
	for (MViewport* pViewport : m_pView->GetViewports())
	{
		pViewport->Render(pRenderer, this);
	}
	m_pView->OnRenderEnd();

}

void MVulkanRenderTarget::OnRenderAfter(MIRenderer* pRenderer)
{
	MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(pRenderer);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore signalSemaphores[] = { pVkRenderer->m_VkRenderFinishedSemaphore };

	uint32_t unFrameIndex = pVkRenderer->GetFrameIndex();

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_VkSwapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &unFrameIndex;
	presentInfo.pResults = nullptr; // Optional
	vkQueuePresentKHR(m_VkPresentQueue, &presentInfo);
}

void MVulkanRenderTarget::Initialize()
{
	InitializeSwapchain();

	m_RenderPass.m_vSubpass.push_back(MSubpass());

	m_RenderPass.m_vBackDesc.push_back(MRenderPass::MRTDesc());
	m_RenderPass.m_vBackDesc.back().bClearWhenRender = true;

	m_RenderPass.m_DepthDesc.bClearWhenRender = true;
}

void MVulkanRenderTarget::Release(MIDevice* pDevice)
{
	m_pDevice->DestroyRenderTarget(this);
	ReleaseSwapchain();

	vkDestroySurfaceKHR(m_pDevice->m_VkInstance, m_VkSurface, nullptr);
}

void MVulkanRenderTarget::Resize(const uint32_t& nWidth, const uint32_t& nHeight)
{
	vkDeviceWaitIdle(m_pDevice->m_VkDevice);

	m_pDevice->DestroyRenderTarget(this);

	ReleaseSwapchain();

	InitializeSwapchain();


	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
}

bool MVulkanRenderTarget::InitializeSwapchain()
{
	VkPhysicalDevice physicalDevice = m_pDevice->GetPhysicalDevice();

	//必须在CreateSpawnchain之前调用。
	int nPresentQueueIndex = m_pDevice->FindQueuePresentFamilies(physicalDevice, m_VkSurface);
	if (nPresentQueueIndex == -1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : nPresentQueueIndex == -1");
		return false;
	}

	VkQueue presentQueue;
	vkGetDeviceQueue(m_pDevice->m_VkDevice, nPresentQueueIndex, 0, &presentQueue);

	VkSurfaceCapabilitiesKHR caps = {};
	VkResult result = m_pDevice->GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_VkSurface, &caps);
	if (result != VK_SUCCESS || caps.maxImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceCapabilitiesKHR error");
		return false;
	}

	VkExtent2D swapchainExtent = {};

	if (caps.currentExtent.width == -1 || caps.currentExtent.height == -1) {
		swapchainExtent.width = m_pView->GetViewWidth();
		swapchainExtent.height = m_pView->GetViewHeight();
	}
	else {
		swapchainExtent = caps.currentExtent;
	}


	uint32_t unPresentModeCount = 0;
	result = m_pDevice->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, NULL);
	if (result != VK_SUCCESS || unPresentModeCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR count < 1");
		return false;
	}

	std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
	result = m_pDevice->GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, vPresentModes.data());

	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR error");
		vkDestroySurfaceKHR(m_pDevice->m_VkInstance, m_VkSurface, nullptr);
		return false;
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (uint32_t i = 0; i < unPresentModeCount; i++) {
		if (vPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}

		if (vPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	uint32_t imageCount = caps.minImageCount + 1;
	if (imageCount > caps.maxImageCount)
		imageCount = caps.maxImageCount;


	uint32_t unFormatCount = 0;
	result = m_pDevice->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_VkSurface, &unFormatCount, NULL);
	if (result != VK_SUCCESS || unFormatCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR unFormatCount < 1");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(unFormatCount);
	result = m_pDevice->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_VkSurface, &unFormatCount, surfaceFormats.data());
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR error");
		return false;
	}

	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;

	if (unFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	else
		colorFormat = surfaceFormats[0].format;
	colorSpace = surfaceFormats[0].colorSpace;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_VkSurface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = colorFormat;
	swapchainCreateInfo.imageColorSpace = colorSpace;
	swapchainCreateInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 1;
	swapchainCreateInfo.pQueueFamilyIndices = { 0 };
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;


	VkSwapchainKHR swapchain;
	result = m_pDevice->CreateSwapchainKHR(m_pDevice->m_VkDevice, &swapchainCreateInfo, NULL, &swapchain);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : CreateSwapchainKHR error");
		return false;
	}

	m_VkSwapchain = swapchain;
	m_VkColorFormat = colorFormat;
	m_VkExtend = swapchainExtent;
	m_VkPresentQueue = presentQueue;

	RebindRenderBuffer();
	return true;
}

void MVulkanRenderTarget::ReleaseSwapchain()
{
	vkDestroySwapchainKHR(m_pDevice->m_VkDevice, m_VkSwapchain, nullptr);
	m_VkSwapchain = VK_NULL_HANDLE;
}

bool MVulkanRenderTarget::RebindRenderBuffer()
{
	this->m_vBackBuffers.clear();
	uint32_t unSwapchainImageCount = 0;
	VkResult result = m_pDevice->GetSwapchainImagesKHR(m_pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, NULL);
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : unSwapchainImageCount < 1");
		return false;
	}

	std::vector<VkImage> vSwapchainImages(unSwapchainImageCount);
	result = m_pDevice->GetSwapchainImagesKHR(m_pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, vSwapchainImages.data());
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetSwapchainImagesKHR error");
		return false;
	}

	//index range is swapchain num
	for (VkImage& image : vSwapchainImages)
	{
		MRenderTextureBuffer* pTextureBuffer = new MRenderTextureBuffer();
		pTextureBuffer->m_VkTextureImage = image;
		pTextureBuffer->m_VkTextureFormat = m_VkColorFormat;
		this->m_vBackBuffers.push_back(pTextureBuffer);
	}

	return true;
}

MVulkanRenderTarget* MVulkanRenderTarget::CreateForWindowsView(MVulkanDevice* pDevice, MWindowsRenderView* pView)
{
	
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = pView->GetHINSTANCE();
	surfaceCreateInfo.hwnd = pView->GetHWND();


	VkSurfaceKHR surface;
	VkResult result = vkCreateWin32SurfaceKHR(pDevice->m_VkInstance, &surfaceCreateInfo, NULL, &surface);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkCreateWin32SurfaceKHR error");
		return nullptr;
	}

	
	MVulkanRenderTarget* pRenderTarget = new MVulkanRenderTarget(pDevice);
	pRenderTarget->m_VkSurface = surface;
	pRenderTarget->m_pView = pView;
	pRenderTarget->Initialize();

	pView->SetRenderTarget(pRenderTarget);

	pRenderTarget->Resize(pView->GetViewWidth(), pView->GetViewHeight());

	return pRenderTarget;
}


VkFramebuffer MVulkanRenderTarget::GetFrameBuffer(const uint32_t& unIndex)
{
	return m_vBackBuffers[unIndex]->m_VkFrameBuffer;
}

#endif