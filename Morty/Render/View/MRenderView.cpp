#include "View/MRenderView.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanRenderCommand.h"
#endif

#include "System/MRenderSystem.h"

MRenderView::MRenderView()
	: m_pEngine(nullptr)
	, m_unWidht(800)
	, m_unHeight(600)
	, m_vRenderTarget()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkSurface = VK_NULL_HANDLE;
	m_VkSwapchain = VK_NULL_HANDLE;

	m_VkPresentQueue = VK_NULL_HANDLE;

	m_unMinImageCount = 0;

	m_VkColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
	m_VkExtend = {};

	m_pDevice = nullptr;
#endif
}

MRenderView::~MRenderView()
{

}

void MRenderView::Resize(const Vector2& v2Size)
{
	vkDeviceWaitIdle(m_pDevice->m_VkDevice);

	m_unWidht = v2Size.x;
	m_unHeight = v2Size.y;

	DestroyRenderPass();
	ReleaseSwapchain();
	InitializeSwapchain();
}

void MRenderView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
}

void MRenderView::Release()
{
	DestroyRenderPass();
	ReleaseSwapchain();


	if (m_VkSurface)
	{
		vkDestroySurfaceKHR(m_pDevice->m_VkInstance, m_VkSurface, nullptr);
		m_VkSurface = VK_NULL_HANDLE;
	}
}

void MRenderView::Present(MRenderTarget* pRenderTarget)
{
	MVulkanPrimaryRenderCommand* pRenderCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderTarget->pPrimaryCommand);
	if (!pRenderCommand)
		return;

	//submit command
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::vector<VkSemaphore> vWaitSemaphoreBeforeSubmit = { pRenderTarget->vkImageReadySemaphore };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = vWaitSemaphoreBeforeSubmit.size();
		submitInfo.pWaitSemaphores = vWaitSemaphoreBeforeSubmit.data();
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		VkCommandBuffer commandBuffers[] = { pRenderCommand->m_VkCommandBuffer };
		//TODO maybe mutil command buffers for every frame
		submitInfo.pCommandBuffers = commandBuffers;

		VkSemaphore signalSemaphores[] = { pRenderCommand->m_VkRenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VkFence vkInFightFence = pRenderCommand->m_VkRenderFinishedFence;
		//m_VkInFlightFences = unsigned
		vkResetFences(m_pDevice->m_VkDevice, 1, &vkInFightFence);
		VkResult success = vkQueueSubmit(m_pDevice->m_VkGraphicsQueue, 1, &submitInfo, vkInFightFence);
		if (success != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}

	// present 
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		std::vector<VkSemaphore> vSignalSemaphores = { pRenderCommand-> m_VkRenderFinishedSemaphore };

		presentInfo.waitSemaphoreCount = vSignalSemaphores.size();
		presentInfo.pWaitSemaphores = vSignalSemaphores.data();
		VkSwapchainKHR swapChains[] = { m_VkSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &pRenderTarget->unImageIndex;
		presentInfo.pResults = nullptr; // Optional
		vkQueuePresentKHR(m_VkPresentQueue, &presentInfo);

		vkDeviceWaitIdle(m_pDevice->m_VkDevice);
	}
}

#if RENDER_GRAPHICS == MORTY_VULKAN
void MRenderView::InitializeForVulkan(MIDevice* pDevice, VkSurfaceKHR surface)
{
	m_pDevice = static_cast<MVulkanDevice*>(pDevice);

	m_VkSurface = surface;

	Resize(Vector2(m_unWidht, m_unHeight));
}

#endif

bool MRenderView::InitializeSwapchain()
{
	VkPhysicalDevice physicalDevice = m_pDevice->GetPhysicalDevice();

	//CreateSpawnchain֮
	int nPresentQueueIndex = m_pDevice->FindQueuePresentFamilies(physicalDevice, m_VkSurface);
	if (nPresentQueueIndex == -1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : nPresentQueueIndex == -1");
		return false;
	}

	VkQueue presentQueue;
	vkGetDeviceQueue(m_pDevice->m_VkDevice, nPresentQueueIndex, 0, &presentQueue);

	VkSurfaceCapabilitiesKHR caps = {};
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_VkSurface, &caps);
	if (result != VK_SUCCESS || caps.maxImageCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceCapabilitiesKHR error");
		return false;
	}

	VkExtent2D swapchainExtent = {};

	if (caps.currentExtent.width == -1 || caps.currentExtent.height == -1) {
		swapchainExtent.width = GetWidth();
		swapchainExtent.height = GetHeight();
	}
	else {
		swapchainExtent = caps.currentExtent;
	}

	if (swapchainExtent.width <= 0 || swapchainExtent.height <= 0)
	{
		GetEngine()->GetLogger()->Warning("Create VulkanRenderTarget Error : swapchain size: (%d, %d)", swapchainExtent.width, swapchainExtent.height);
		return false;
	}

	uint32_t unPresentModeCount = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, NULL);
	if (result != VK_SUCCESS || unPresentModeCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR count < 1");
		return false;
	}

	std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, vPresentModes.data());

	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR error");
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
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_VkSurface, &unFormatCount, NULL);
	if (result != VK_SUCCESS || unFormatCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR unFormatCount < 1");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(unFormatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_VkSurface, &unFormatCount, surfaceFormats.data());
	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR error");
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
	result = vkCreateSwapchainKHR(m_pDevice->m_VkDevice, &swapchainCreateInfo, NULL, &swapchain);
	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : CreateSwapchainKHR error");
		return false;
	}

	m_unMinImageCount = imageCount;
	m_VkSwapchain = swapchain;
	m_VkColorFormat = colorFormat;
	m_VkExtend = swapchainExtent;
	m_VkPresentQueue = presentQueue;

	BindRenderPass();
	return true;
}

void MRenderView::ReleaseSwapchain()
{
	if (VK_NULL_HANDLE != m_VkSwapchain)
	{
		vkDestroySwapchainKHR(m_pDevice->m_VkDevice, m_VkSwapchain, nullptr);
		m_VkSwapchain = VK_NULL_HANDLE;
	}
}

bool MRenderView::BindRenderPass()
{
	DestroyRenderPass();

	uint32_t unSwapchainImageCount = 0;
	VkResult result = vkGetSwapchainImagesKHR(m_pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, NULL);
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : unSwapchainImageCount < 1");
		return false;
	}

	std::vector<VkImage> vSwapchainImages(unSwapchainImageCount);
	result = vkGetSwapchainImagesKHR(m_pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, vSwapchainImages.data());
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetSwapchainImagesKHR error");
		return false;
	}

	//index range is swapchain num

	Vector2 size(GetWidth(), GetHeight());
	m_vRenderTarget.resize(vSwapchainImages.size());
	for (size_t i = 0; i < vSwapchainImages.size(); ++i)
	{
		m_vRenderTarget[i].unImageIndex = i;
		m_vRenderTarget[i].pPrimaryCommand = nullptr;
		m_vRenderTarget[i].vkImageReadySemaphore = VK_NULL_HANDLE;

		MTexture* pTexture = new MTexture();
		pTexture->SetName("Editor Render View");
		pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
		pTexture->SetRenderUsage(METextureRenderUsage::ERenderPresent);
		pTexture->SetSize(size);
		pTexture->m_VkTextureImage = vSwapchainImages[i];
		pTexture->m_VkTextureImageMemory = VK_NULL_HANDLE;
		pTexture->m_VkImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		pTexture->m_VkTextureFormat = m_VkColorFormat;
		pTexture->GenerateBuffer(m_pDevice);
		m_vRenderTarget[i].renderPass.AddBackTexture(pTexture, { true, MColor::Black_T });

		MTexture* pDepthTexture = new MTexture();
		pDepthTexture->SetName("Editor Depth View");
		pDepthTexture->SetTextureLayout(METextureLayout::EDepth);
		pDepthTexture->SetRenderUsage(METextureRenderUsage::ERenderDepth);
		pDepthTexture->SetSize(size);
		pDepthTexture->GenerateBuffer(m_pDevice);
		m_vRenderTarget[i].renderPass.SetDepthTexture(pDepthTexture, { true, MColor::White });
		m_vRenderTarget[i].renderPass.m_vkExtent2D = m_VkExtend;
		m_vRenderTarget[i].renderPass.GenerateBuffer(m_pDevice);
	}


	return true;
}

void MRenderView::DestroyRenderPass()
{

	for (MRenderTarget rendertarget : m_vRenderTarget)
	{
		if (rendertarget.vkImageReadySemaphore)
		{
			m_pDevice->GetRecycleBin()->DestroySemaphoreLater(rendertarget.vkImageReadySemaphore);
			rendertarget.vkImageReadySemaphore = VK_NULL_HANDLE;
		}

		for (MBackTexture& tex : rendertarget.renderPass.m_vBackTextures)
		{
			tex.pTexture->DestroyBuffer(m_pDevice);
			delete tex.pTexture;
			tex.pTexture = nullptr;
		}

		rendertarget.renderPass.m_vBackTextures.clear();

		if (MTexture* pDepthTexture = rendertarget.renderPass.GetDepthTexture())
		{
			pDepthTexture->DestroyBuffer(m_pDevice);
			delete pDepthTexture;
			pDepthTexture = nullptr;
			rendertarget.renderPass.SetDepthTexture(nullptr, {});
		}

		rendertarget.renderPass.DestroyBuffer(m_pDevice);
		rendertarget.pPrimaryCommand = nullptr;
	}

	m_vRenderTarget.clear();
}

MRenderTarget* MRenderView::GetNextRenderTarget()
{
	if (VK_NULL_HANDLE == m_VkSwapchain)
		return nullptr;

	//get available image. by semaphore and index.
	uint32_t unImageIndex = 0;
	VkSemaphore vkImageReadySemaphore = VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	MORTY_ASSERT(vkCreateSemaphore(m_pDevice->m_VkDevice, &semaphoreInfo, nullptr, &vkImageReadySemaphore) == VK_SUCCESS);

	VkResult result = vkAcquireNextImageKHR(m_pDevice->m_VkDevice, m_VkSwapchain, UINT64_MAX, vkImageReadySemaphore, VK_NULL_HANDLE, &unImageIndex);
	
	if (result != VK_SUCCESS)
	{
		return nullptr;// windows minimize or other, don`t render.
	}

	if (m_vRenderTarget[unImageIndex].vkImageReadySemaphore)
	{
		m_pDevice->GetRecycleBin()->DestroySemaphoreLater(m_vRenderTarget[unImageIndex].vkImageReadySemaphore);
		m_vRenderTarget[unImageIndex].vkImageReadySemaphore = nullptr;
	}

	m_vRenderTarget[unImageIndex].vkImageReadySemaphore = vkImageReadySemaphore;

//	GetEngine()->GetLogger()->Information("the Image View: %d", unImageIndex);

	//now, framebuffer is ready, commandbuffer is ready.
	return &m_vRenderTarget[unImageIndex];
}
