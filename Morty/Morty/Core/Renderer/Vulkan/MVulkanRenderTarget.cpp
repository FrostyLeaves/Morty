#include "MVulkanRenderTarget.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MIRenderView.h"
#include "MVulkanRenderer.h"

M_OBJECT_IMPLEMENT(MVulkanRenderTarget, MIRenderTarget)

#if RENDER_GRAPHICS == MORTY_VULKAN

#ifdef MORTY_WIN
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#include "DirectX11/MWindowsRenderView.h"
#endif

#ifdef MORTY_ANDROID
#include "vulkan/vulkan_android.h"
#include "MAndroidRenderView.h"
#endif

MVulkanRenderTarget::MVulkanRenderTarget()
	: MIRenderTarget()
	, m_pView(nullptr)
	, m_VkSurface(VK_NULL_HANDLE)
	, m_VkSwapchain(VK_NULL_HANDLE)
	, m_VkPresentQueue(VK_NULL_HANDLE)
	, m_unFrameBufferIndex(0)
	, m_VkImageAvailableSemaphore(VK_NULL_HANDLE)
	, m_unMinImageCount(0)
	, m_RenderPass()
{

}

MVulkanRenderTarget::~MVulkanRenderTarget()
{

}

void MVulkanRenderTarget::OnRender(MIRenderer* pRenderer)
{

}

void MVulkanRenderTarget::OnRenderBefore(MIRenderer* pRenderer)
{
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(GetEngine()->GetDevice());
	if (!pDevice)
		return;

	vkAcquireNextImageKHR(pDevice->m_VkDevice, m_VkSwapchain, UINT64_MAX, m_VkImageAvailableSemaphore, VK_NULL_HANDLE, &m_unFrameBufferIndex);
}

void MVulkanRenderTarget::OnRenderAfter(MIRenderer* pRenderer)
{
	MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(pRenderer);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	uint32_t unFrameIndex = pVkRenderer->GetFrameIndex();
	VkSemaphore signalSemaphores[] = { m_aVkRenderFinishedSemaphore[unFrameIndex] };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_VkSwapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_unFrameBufferIndex;
	presentInfo.pResults = nullptr; // Optional
	vkQueuePresentKHR(m_VkPresentQueue, &presentInfo);
}

void MVulkanRenderTarget::OnCreated()
{
	Super::OnCreated();
}

void MVulkanRenderTarget::Initialize()
{
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(GetEngine()->GetDevice());
	if (!pDevice)
	{
		MLogManager::GetInstance()->Error("MVulkanRenderTarget::Initialize Error: device == nullptr");
		return;
	}

	m_RenderPass.m_vBackDesc.push_back({});
	m_RenderPass.m_vBackDesc.back().bClearWhenRender = true;
	m_RenderPass.m_DepthDesc.bClearWhenRender = true;

	InitializeSwapchain(pDevice);

	//Create WaitSemaphore
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(pDevice->m_VkDevice, &semaphoreInfo, nullptr, &m_VkImageAvailableSemaphore) != VK_SUCCESS)
		return;

	m_vWaitSemaphoreBeforeSubmit.push_back(m_VkImageAvailableSemaphore);
}

void MVulkanRenderTarget::OnDelete()
{
	Super::OnDelete();

	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(GetEngine()->GetDevice());
	if (!pDevice)
	{
		MLogManager::GetInstance()->Error("MVulkanRenderTarget::Release Error: device == nullptr");
		return;
	}


	CleanRenderBuffer(pDevice);
	pDevice->DestroyRenderTarget(this);

	ReleaseSwapchain(pDevice);

	vkDestroySemaphore(pDevice->m_VkDevice, m_VkImageAvailableSemaphore, nullptr);
	m_VkImageAvailableSemaphore = VK_NULL_HANDLE;

	m_vWaitSemaphoreBeforeSubmit.clear();

	vkDestroySurfaceKHR(pDevice->m_VkInstance, m_VkSurface, nullptr);
}

void MVulkanRenderTarget::Resize(const Vector2& v2Size)
{
	Super::Resize(v2Size);

	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(GetEngine()->GetDevice());
	if (!pDevice)
	{
		MLogManager::GetInstance()->Error("MVulkanRenderTarget::Resize Error: device == nullptr");
		return;
	}

	vkDeviceWaitIdle(pDevice->m_VkDevice);


	CleanRenderBuffer(pDevice);
// 
 	pDevice->DestroyRenderTarget(this);
	pDevice->GenerateRenderTarget(&m_RenderPass, this);

	ReleaseSwapchain(pDevice);

	InitializeSwapchain(pDevice);
}

uint32_t MVulkanRenderTarget::GetFrameBufferIndex()
{
	return m_unFrameBufferIndex;
}

bool MVulkanRenderTarget::InitializeSwapchain(MVulkanDevice* pDevice)
{
	VkPhysicalDevice physicalDevice = pDevice->GetPhysicalDevice();

	//������CreateSpawnchain֮ǰ���á�
	int nPresentQueueIndex = pDevice->FindQueuePresentFamilies(physicalDevice, m_VkSurface);
	if (nPresentQueueIndex == -1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : nPresentQueueIndex == -1");
		return false;
	}

	VkQueue presentQueue;
	vkGetDeviceQueue(pDevice->m_VkDevice, nPresentQueueIndex, 0, &presentQueue);

	VkSurfaceCapabilitiesKHR caps = {};
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_VkSurface, &caps);
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
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, NULL);
	if (result != VK_SUCCESS || unPresentModeCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR count < 1");
		return false;
	}

	std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_VkSurface, &unPresentModeCount, vPresentModes.data());

	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkGetPhysicalDeviceSurfacePresentModesKHR error");
		vkDestroySurfaceKHR(pDevice->m_VkInstance, m_VkSurface, nullptr);
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
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR unFormatCount < 1");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(unFormatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_VkSurface, &unFormatCount, surfaceFormats.data());
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
	result = vkCreateSwapchainKHR(pDevice->m_VkDevice, &swapchainCreateInfo, NULL, &swapchain);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : CreateSwapchainKHR error");
		return false;
	}

	m_unMinImageCount = imageCount;
	m_VkSwapchain = swapchain;
	m_VkColorFormat = colorFormat;
	m_VkExtend = swapchainExtent;
	m_VkPresentQueue = presentQueue;

	RebindRenderBuffer(pDevice);
	return true;
}

void MVulkanRenderTarget::ReleaseSwapchain(MVulkanDevice* pDevice)
{
	vkDestroySwapchainKHR(pDevice->m_VkDevice, m_VkSwapchain, nullptr);
	m_VkSwapchain = VK_NULL_HANDLE;
}

void MVulkanRenderTarget::CleanRenderBuffer(MVulkanDevice* pDevice)
{
	MFrameBuffer* pFrameBuffer = m_RenderPass.GetFrameBuffer();
	for (MIRenderTexture* pTexture : pFrameBuffer->vBackTextures)
	{
		pTexture->DestroyBuffer(pDevice);
		delete pTexture;
	}

	pFrameBuffer->vBackTextures.clear();

	if (pFrameBuffer->pDepthTexture)
	{
		pFrameBuffer->pDepthTexture->DestroyBuffer(pDevice);
		delete pFrameBuffer->pDepthTexture;
		pFrameBuffer->pDepthTexture = nullptr;
	}

	GetEngine()->GetDevice()->DestroyFrameBuffer(&m_RenderPass);
	GetEngine()->GetDevice()->DestroyRenderPass(&m_RenderPass);
}

bool MVulkanRenderTarget::RebindRenderBuffer(MVulkanDevice* pDevice)
{
	CleanRenderBuffer(pDevice);

	uint32_t unSwapchainImageCount = 0;
	VkResult result = vkGetSwapchainImagesKHR(pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, NULL);
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : unSwapchainImageCount < 1");
		return false;
	}

	std::vector<VkImage> vSwapchainImages(unSwapchainImageCount);
	result = vkGetSwapchainImagesKHR(pDevice->m_VkDevice, m_VkSwapchain, &unSwapchainImageCount, vSwapchainImages.data());
	if (result != VK_SUCCESS || unSwapchainImageCount < 1)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : GetSwapchainImagesKHR error");
		return false;
	}

	//index range is swapchain num

	MFrameBuffer* pFrameBuffer = m_RenderPass.GetFrameBuffer();

	MRenderSwapchainTexture* pTexture = new MRenderSwapchainTexture(unSwapchainImageCount);
	for (size_t i = 0; i < vSwapchainImages.size(); ++i)
	{
		MTextureBuffer* pBuffer = pTexture->GetBuffer(i);
		pBuffer->m_VkTextureImage = vSwapchainImages[i];
		pBuffer->m_VkTextureImageMemory = VK_NULL_HANDLE;
		pBuffer->m_VkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		pBuffer->m_VkTextureFormat = m_VkColorFormat;
	}
	pTexture->SetSize(m_v2Size);
	pTexture->GenerateBuffer(pDevice);
	pFrameBuffer->vBackTextures.push_back(pTexture);

	MRenderTexture* pDepthTexture = new MRenderTexture();
	pDepthTexture->SetType(METextureLayout::EDepth);
	pDepthTexture->SetUsage(METextureUsage::ERenderDepth);
	pDepthTexture->SetSize(m_v2Size);
	pDepthTexture->GenerateBuffer(pDevice);
	pFrameBuffer->pDepthTexture = pDepthTexture;

	pFrameBuffer->m_vkExtent2D = m_VkExtend;

	return true;
}

MVulkanRenderTarget* MVulkanRenderTarget::CreateForSurface(MEngine* pEngine, MIRenderView* pView, VkSurfaceKHR surface)
{
	MVulkanRenderTarget* pRenderTarget = pEngine->GetObjectManager()->CreateObject<MVulkanRenderTarget>();
	pRenderTarget->m_VkSurface = surface;
	pRenderTarget->m_pView = pView;
	pRenderTarget->Initialize();

	pView->SetRenderTarget(pRenderTarget);

	pRenderTarget->Resize(Vector2(pView->GetViewWidth(), pView->GetViewHeight()));

	return pRenderTarget;
}

#ifdef MORTY_WIN
MVulkanRenderTarget* MVulkanRenderTarget::CreateForWindowsView(MEngine* pEngine, MIRenderView* pView)
{
	MVulkanDevice* pVkDevice = dynamic_cast<MVulkanDevice*>(pEngine->GetDevice());
	if (nullptr == pVkDevice)
		return nullptr;

	MWindowsRenderView* pWinView = dynamic_cast<MWindowsRenderView*>(pView);
	if (nullptr == pWinView)
		return nullptr;

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = pWinView->GetHINSTANCE();
	surfaceCreateInfo.hwnd = pWinView->GetHWND();


	VkSurfaceKHR surface;
	VkResult result = vkCreateWin32SurfaceKHR(pVkDevice->m_VkInstance, &surfaceCreateInfo, NULL, &surface);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkCreateWin32SurfaceKHR error");
		return nullptr;
	}
	
	return CreateForSurface(pEngine, pView, surface);
}

#endif

#ifdef MORTY_ANDROID
MVulkanRenderTarget* MVulkanRenderTarget::CreateForAndroidView(MEngine* pEngine, MIRenderView* pView)
{
	MVulkanDevice* pVkDevice = dynamic_cast<MVulkanDevice*>(pDevice);
	if (nullptr == pVkDevice)
	{
		MLogManager::GetInstance()->Error("CreateForAndroidView Error: Device not matching.");
		return nullptr;
	}

	MAndroidRenderView* pAndView = dynamic_cast<MAndroidRenderView*>(pView);
	if (nullptr == pAndView)
	{
		MLogManager::GetInstance()->Error("CreateForAndroidView Error: RenderView not matching.");
		return nullptr;
	}

	VkAndroidSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.window = pAndView->GetNativeWindow();

	VkSurfaceKHR surface;
	MLogManager::GetInstance()->Information("CreateForAndroidView: Test to call vkCreateAndroidSurfaceKHR");
	VkResult result = vkCreateAndroidSurfaceKHR(pVkDevice->m_VkInstance, &createInfo, nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		MLogManager::GetInstance()->Error("Create VulkanRenderTarget Error : vkCreateAndroidSurfaceKHR error");
		return nullptr;
	}

	return CreateForSurface(pEngine, pView, surface);
}

#endif


#endif
