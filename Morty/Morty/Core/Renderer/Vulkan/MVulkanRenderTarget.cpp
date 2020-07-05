#include "MVulkanRenderTarget.h"

#include "MViewport.h"
#include "MVulkanRenderer.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#ifdef MORTY_WIN
#include "vulkan/vulkan_win32.h"
#endif

MVulkanRenderTarget::MVulkanRenderTarget(MVulkanDevice* pDevice)
	:MIRenderTarget()
	, m_pDevice(pDevice)
	, m_pView(nullptr)
{

}

MVulkanRenderTarget::~MVulkanRenderTarget()
{

}

MRenderTargetTexture* MVulkanRenderTarget::GetBackTexture(const uint32_t& unIndex)
{
	return &m_vRenderTargets[unIndex];
}

void MVulkanRenderTarget::OnRender(MIRenderer* pRenderer)
{
	MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(pRenderer);

	//TODO ²»ÓÅÑÅ
	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_pDevice->m_VkDevice, m_VkSwapchain, UINT64_MAX, pVkRenderer->m_VkImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	pVkRenderer->SetFrameIndex(imageIndex);

	pRenderer->ClearDepthTexture(GetDepthTexture());
	pRenderer->ClearRenderTargetView(&this->m_vRenderTargets[imageIndex], m_pView->GetBackColor());

// 	m_pView->OnRenderBegin();
// 	for (MViewport* pViewport : m_pView->GetViewports())
// 	{
// 		pViewport->Render(pRenderer, this);
// 	}
// 	m_pView->OnRenderEnd();



	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore signalSemaphores[] = { pVkRenderer->m_VkRenderFinishedSemaphore };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_VkSwapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	vkQueuePresentKHR(m_VkPresentQueue, &presentInfo);
}

void MVulkanRenderTarget::Release(MIDevice* pDevice)
{
	vkDestroySurfaceKHR(m_pDevice->m_VkInstance, m_VkSurface, nullptr);
}

void MVulkanRenderTarget::Resize(const uint32_t& nWidth, const uint32_t& nHeight)
{
	m_pDevice->DestroyRenderTarget(this);


	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
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

	VkSurfaceCapabilitiesKHR caps = {};
	result = pDevice->GetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice->GetPhysicalDevice(), surface, &caps);
	if (result != VK_SUCCESS || caps.maxImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceCapabilitiesKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
	}

	VkExtent2D swapchainExtent = {};

	if (caps.currentExtent.width == -1 || caps.currentExtent.height == -1) {
		swapchainExtent.width = pView->GetViewWidth();
		swapchainExtent.height = pView->GetViewHeight();
	}
	else {
		swapchainExtent = caps.currentExtent;
	}


	uint32_t unPresentModeCount = 0;
	result = pDevice->GetPhysicalDeviceSurfacePresentModesKHR(pDevice->GetPhysicalDevice(), surface, &unPresentModeCount, NULL);
	if (result != VK_SUCCESS || unPresentModeCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR count < 1");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
	}

	std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
	result = pDevice->GetPhysicalDeviceSurfacePresentModesKHR(pDevice->GetPhysicalDevice(), surface, &unPresentModeCount, vPresentModes.data());

	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
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
	result = pDevice->GetPhysicalDeviceSurfaceFormatsKHR(pDevice->GetPhysicalDevice(), surface, &unFormatCount, NULL);
	if (result != VK_SUCCESS || unFormatCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR unFormatCount < 1");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(unFormatCount);
	result = pDevice->GetPhysicalDeviceSurfaceFormatsKHR(pDevice->GetPhysicalDevice(), surface, &unFormatCount, surfaceFormats.data());
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
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
	swapchainCreateInfo.surface = surface;
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
	result = pDevice->CreateSwapchainKHR(pDevice->m_VkDevice, &swapchainCreateInfo, NULL, &swapchain);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : CreateSwapchainKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		return nullptr;
	}

	uint32_t unSwapchainImageCount = 0;
	result = pDevice->GetSwapchainImagesKHR(pDevice->m_VkDevice, swapchain, &unSwapchainImageCount, NULL);
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : unSwapchainImageCount < 1");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		vkDestroySwapchainKHR(pDevice->m_VkDevice, swapchain, nullptr);
		return nullptr;
	}

	std::vector<VkImage> vSwapchainImages(unSwapchainImageCount);
	result = pDevice->GetSwapchainImagesKHR(pDevice->m_VkDevice, swapchain, &unSwapchainImageCount, vSwapchainImages.data());
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetSwapchainImagesKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		vkDestroySwapchainKHR(pDevice->m_VkDevice, swapchain, nullptr);
		return nullptr;
	}



	MVulkanRenderTarget* pRenderTarget = new MVulkanRenderTarget(pDevice);
	pRenderTarget->m_VkSwapchain = swapchain;
	pRenderTarget->m_VkSurface = surface;
	pRenderTarget->m_VkColorFormat = colorFormat;
	pRenderTarget->m_vSwapchainImages = vSwapchainImages;
	pRenderTarget->m_pView = pView;
	pRenderTarget->m_VkExtend = swapchainExtent;
	pView->SetRenderTarget(pRenderTarget);


	int nPresentQueueIndex = pDevice->FindQueuePresentFamilies(pDevice->GetPhysicalDevice(), surface);
	if (nPresentQueueIndex == -1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : nPresentQueueIndex == -1");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, surface, nullptr);
		vkDestroySwapchainKHR(pDevice->m_VkDevice, swapchain, nullptr);
		return nullptr;
	}
	vkGetDeviceQueue(pDevice->m_VkDevice, nPresentQueueIndex, 0, &pRenderTarget->m_VkPresentQueue);

	pRenderTarget->Resize(pView->GetViewWidth(), pView->GetViewHeight());

	return pRenderTarget;
}


#endif