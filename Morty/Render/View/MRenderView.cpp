#include "View/MRenderView.h"
#include "Utility/MGlobal.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanRenderCommand.h"
#endif

#include "Render/Vulkan/MVulkanPhysicalDevice.h"
#include "System/MRenderSystem.h"
#include "Utility/MFunction.h"

using namespace morty;

void MViewRenderTarget::BindPrimaryCommand(MIRenderCommand* pCommand)
{
	pPrimaryCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pCommand);
	pPrimaryCommand->m_vRenderWaitSemaphore.push_back(vkImageReadySemaphore);
}

MRenderView::MRenderView()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkSurface = VK_NULL_HANDLE;
	m_VkSwapchain = VK_NULL_HANDLE;

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
	//vkDeviceWaitIdle(m_pDevice->m_VkDevice);
	
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
	//wait for prev submit finished. 
	while (!m_bSubmitFinished) {}

	DestroyRenderPass();
	ReleaseSwapchain();


	if (m_VkSurface)
	{
		vkDestroySurfaceKHR(m_pDevice->GetVkInstance(), m_VkSurface, nullptr);
		m_VkSurface = VK_NULL_HANDLE;
	}
}

void MRenderView::PresetWork(MViewRenderTarget* pRenderTarget)
{
	MVulkanPrimaryRenderCommand* pRenderCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderTarget->pPrimaryCommand);
	MORTY_ASSERT(pRenderCommand);

	// present 
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		std::vector<VkSemaphore> vSignalSemaphores = { pRenderCommand->m_VkRenderFinishedSemaphore };

		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vSignalSemaphores.size());
		presentInfo.pWaitSemaphores = vSignalSemaphores.data();
		VkSwapchainKHR swapChains[] = { m_VkSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &pRenderTarget->unImageIndex;
		presentInfo.pResults = nullptr; // Optional
		vkQueuePresentKHR(m_pDevice->m_VkPresetQueue, &presentInfo);

		const VkResult result = vkQueueWaitIdle(m_pDevice->m_VkPresetQueue);
		MORTY_ASSERT(result == VK_SUCCESS);
	}

	m_bSubmitFinished = true;
}

void MRenderView::Present(MViewRenderTarget* pRenderTarget)
{

	//wait for prev submit finished. 
	while (!m_bSubmitFinished) {}

	m_bSubmitFinished = false;


	// submit
	MVulkanPrimaryRenderCommand* pRenderCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderTarget->pPrimaryCommand);
	MORTY_ASSERT(pRenderCommand);
	m_pDevice->SubmitCommand(pRenderCommand);

	// preset
	MThreadWork presetWork;
	presetWork.funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(MRenderView::PresetWork, this, pRenderTarget);
	presetWork.eThreadType = MRenderGlobal::THREAD_ID_SUBMIT;

	GetEngine()->GetThreadPool()->AddWork(presetWork);
	
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
	VkPhysicalDevice physicalDevice = m_pDevice->GetPhysicalDevice()->m_VkPhysicalDevice;

	//CreateSpawnchain֮
	int nPresentQueueIndex = m_pDevice->FindQueuePresentFamilies(m_VkSurface);
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

	if (caps.currentExtent.width == MGlobal::M_INVALID_UINDEX || caps.currentExtent.height == MGlobal::M_INVALID_UINDEX) {
		swapchainExtent.width = GetWidth();
		swapchainExtent.height = GetHeight();
	}
	else {
		swapchainExtent = caps.currentExtent;
	}

	if (swapchainExtent.width <= 0 || swapchainExtent.height <= 0)
	{
		GetEngine()->GetLogger()->Warning("Create VulkanRenderTarget Error : swapchain size: ({}, {})", swapchainExtent.width, swapchainExtent.height);
		return false;
	}

	m_unWidht = std::min(m_unWidht, swapchainExtent.width);
	m_unHeight = std::min(m_unHeight, swapchainExtent.height);

	uint32_t unPresentModeCount = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, NULL);
	if (result != VK_SUCCESS || unPresentModeCount < 1)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : result: {}, vkGetPhysicalDeviceSurfacePresentModesKHR count {}", (int)result, unPresentModeCount);
		return false;
	}

	std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, vPresentModes.data());

	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR error");
		vkDestroySurfaceKHR(m_pDevice->GetVkInstance(), m_VkSurface, nullptr);
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


	VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_VkSurface, &vkSurfaceCapabilities);

	std::vector<VkCompositeAlphaFlagBitsKHR> vDefaultCompositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
	    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
	    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};

	VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlag = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;

	for (auto flag : vDefaultCompositeAlphaFlags)
	{
        auto support = flag & vkSurfaceCapabilities.supportedCompositeAlpha;
		if (support != 0)
		{
			vkCompositeAlphaFlag = static_cast<VkCompositeAlphaFlagBitsKHR>(flag);
			break;
		}
	}

	if (vkCompositeAlphaFlag == VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR)
	{
		MORTY_ASSERT(vkCompositeAlphaFlag != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);
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
	swapchainCreateInfo.compositeAlpha = vkCompositeAlphaFlag;
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

	Vector2i size(GetWidth(), GetHeight());
	m_vRenderTarget.resize(vSwapchainImages.size());
	for (size_t i = 0; i < vSwapchainImages.size(); ++i)
	{
		m_vRenderTarget[i].unImageIndex = static_cast<uint32_t>(i);
		m_vRenderTarget[i].pPrimaryCommand = nullptr;
		m_vRenderTarget[i].vkImageReadySemaphore = VK_NULL_HANDLE;

		std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
		pTexture->SetName("Editor Render View");
		pTexture->SetTextureLayout(METextureLayout::UNorm_RGBA8);
		pTexture->SetRenderUsage(METextureWriteUsage::ERenderPresent);
		pTexture->SetSize(size);
		pTexture->m_VkTextureImage = vSwapchainImages[i];
		pTexture->m_VkTextureImageMemory = VK_NULL_HANDLE;
		pTexture->m_VkImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		pTexture->m_VkTextureFormat = m_VkColorFormat;
		pTexture->GenerateBuffer(m_pDevice);
		m_vRenderTarget[i].renderPass.AddBackTexture(pTexture, { true, MColor::Black_T });

		std::shared_ptr<MTexture> pDepthTexture = MTexture::CreateTexture(
			MTexture::CreateDepthBuffer()
			.InitName("Editor Depth View")
			.InitSize(size)
		);

		pDepthTexture->GenerateBuffer(m_pDevice);
		m_vRenderTarget[i].renderPass.SetDepthTexture(pDepthTexture, { true, MColor::White });
		m_vRenderTarget[i].renderPass.m_vkExtent2D = m_VkExtend;
		m_vRenderTarget[i].renderPass.GenerateBuffer(m_pDevice);
	}


	return true;
}

void MRenderView::DestroyRenderPass()
{
	for (MViewRenderTarget rendertarget : m_vRenderTarget)
	{
		if (rendertarget.vkImageReadySemaphore)
		{
			m_pDevice->GetRecycleBin()->DestroySemaphoreLater(rendertarget.vkImageReadySemaphore);
			rendertarget.vkImageReadySemaphore = VK_NULL_HANDLE;
		}

		for (MRenderTarget& tex : rendertarget.renderPass.m_renderTarget.backTargets)
		{
			tex.pTexture->DestroyBuffer(m_pDevice);
			tex.pTexture = nullptr;
		}

		rendertarget.renderPass.m_renderTarget.backTargets.clear();

		if (std::shared_ptr<MTexture> pDepthTexture = rendertarget.renderPass.GetDepthTexture())
		{
			pDepthTexture->DestroyBuffer(m_pDevice);
			rendertarget.renderPass.SetDepthTexture(nullptr, {});
		}

		rendertarget.renderPass.DestroyBuffer(m_pDevice);
		rendertarget.pPrimaryCommand = nullptr;
	}

	m_vRenderTarget.clear();
}

MViewRenderTarget* MRenderView::GetNextRenderTarget()
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

//	GetEngine()->GetLogger()->Information("the Image View: {}", unImageIndex);

	//now, framebuffer is ready, commandbuffer is ready.
	return &m_vRenderTarget[unImageIndex];
}
