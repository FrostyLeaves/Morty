#include "View/MRenderView.h"
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanRenderCommand.h"

#endif

#include "RHI/Vulkan/MTextureRHIVulkan.h"
#include "RHI/Vulkan/MVulkanPhysicalDevice.h"
#include "System/MRenderSystem.h"
#include "Utility/MFunction.h"

using namespace morty;

void MViewRenderTarget::BindPrimaryCommand(MIRenderCommand* pCommand)
{
    pPrimaryCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pCommand);
    pPrimaryCommand->m_renderWaitSemaphore.push_back(vkImageReadySemaphore);
}

MRenderView::MRenderView()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkSurface   = VK_NULL_HANDLE;
    m_vkSwapchain = VK_NULL_HANDLE;

    m_unMinImageCount = 0;

    m_vkColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
    m_vkExtend      = {};

    m_device = nullptr;
#endif
}

MRenderView::~MRenderView() {}

void MRenderView::Resize(const Vector2& v2Size)
{
    //vkDeviceWaitIdle(m_device->m_vkDevice);

    m_widht  = v2Size.x;
    m_height = v2Size.y;

    DestroyRenderPass();
    ReleaseSwapchain();
    InitializeSwapchain();
}

void MRenderView::Initialize(MEngine* pEngine) { m_engine = pEngine; }

void MRenderView::InitSize(uint32_t nWidht, uint32_t nHeight)
{
    MORTY_ASSERT(m_vkSwapchain == VK_NULL_HANDLE);

    m_widht  = nWidht;
    m_height = nHeight;
}

void MRenderView::Release()
{
    //wait for prev submit finished.
    while (!m_submitFinished) {}

    DestroyRenderPass();
    ReleaseSwapchain();


    if (m_vkSurface)
    {
        vkDestroySurfaceKHR(m_device->GetVkInstance(), m_vkSurface, nullptr);
        m_vkSurface = VK_NULL_HANDLE;
    }
}

void MRenderView::PresetWork(MViewRenderTarget* pRenderTarget)
{
    MVulkanPrimaryRenderCommand* pRenderCommand =
            dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderTarget->pPrimaryCommand);
    MORTY_ASSERT(pRenderCommand);

    // present
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        std::vector<VkSemaphore> vSignalSemaphores = {pRenderCommand->m_vkRenderFinishedSemaphore};

        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(vSignalSemaphores.size());
        presentInfo.pWaitSemaphores    = vSignalSemaphores.data();
        VkSwapchainKHR swapChains[]    = {m_vkSwapchain};
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &pRenderTarget->unImageIndex;
        presentInfo.pResults           = nullptr;// Optional
        vkQueuePresentKHR(m_device->m_vkPresetQueue, &presentInfo);

        const VkResult result = vkQueueWaitIdle(m_device->m_vkPresetQueue);
        MORTY_ASSERT(result == VK_SUCCESS);
    }

    m_submitFinished = true;
}

void MRenderView::Present(MViewRenderTarget* pRenderTarget)
{

    //wait for prev submit finished.
    while (!m_submitFinished) {}

    m_submitFinished = false;


    // submit
    MVulkanPrimaryRenderCommand* pRenderCommand =
            dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderTarget->pPrimaryCommand);
    MORTY_ASSERT(pRenderCommand);
    m_device->SubmitCommand(pRenderCommand);

    // preset
    MThreadWork presetWork;
    presetWork.funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(MRenderView::PresetWork, this, pRenderTarget);
    presetWork.eThreadType      = MRenderGlobal::THREAD_ID_SUBMIT;

    GetEngine()->GetThreadPool()->AddWork(presetWork);
}

#if RENDER_GRAPHICS == MORTY_VULKAN

void MRenderView::InitializeForVulkan(MIDevice* pDevice, VkSurfaceKHR surface)
{
    m_device = static_cast<MVulkanDevice*>(pDevice);

    m_vkSurface = surface;

    Resize(Vector2(m_widht, m_height));
}

#endif

bool MRenderView::InitializeSwapchain()
{
    VkPhysicalDevice physicalDevice = m_device->GetPhysicalDevice()->m_vkPhysicalDevice;

    //CreateSpawnchain֮
    int              nPresentQueueIndex = m_device->FindQueuePresentFamilies(m_vkSurface);
    if (nPresentQueueIndex == -1)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : nPresentQueueIndex == -1");
        return false;
    }

    VkQueue presentQueue;
    vkGetDeviceQueue(m_device->m_vkDevice, nPresentQueueIndex, 0, &presentQueue);

    VkSurfaceCapabilitiesKHR caps   = {};
    VkResult                 result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_vkSurface, &caps);
    if (result != VK_SUCCESS || caps.maxImageCount < 1)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : "
                                        "GetPhysicalDeviceSurfaceCapabilitiesKHR error");
        return false;
    }

    VkExtent2D swapchainExtent = {};

    if (caps.currentExtent.width == MGlobal::M_INVALID_UINDEX || caps.currentExtent.height == MGlobal::M_INVALID_UINDEX)
    {
        swapchainExtent.width  = GetWidth();
        swapchainExtent.height = GetHeight();
    }
    else { swapchainExtent = caps.currentExtent; }

    if (swapchainExtent.width <= 0 || swapchainExtent.height <= 0)
    {
        GetEngine()->GetLogger()->Warning(
                "Create VulkanRenderTarget Error : swapchain size: ({}, {})",
                swapchainExtent.width,
                swapchainExtent.height
        );
        return false;
    }

    m_widht  = std::min(m_widht, swapchainExtent.width);
    m_height = std::min(m_height, swapchainExtent.height);

    uint32_t unPresentModeCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_vkSurface, &unPresentModeCount, NULL);
    if (result != VK_SUCCESS || unPresentModeCount < 1)
    {
        GetEngine()->GetLogger()->Error(
                "Create VulkanRenderTarget Error : result: {}, "
                "vkGetPhysicalDeviceSurfacePresentModesKHR count {}",
                (int) result,
                unPresentModeCount
        );
        return false;
    }

    std::vector<VkPresentModeKHR> vPresentModes(unPresentModeCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            m_vkSurface,
            &unPresentModeCount,
            vPresentModes.data()
    );

    if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : "
                                        "vkGetPhysicalDeviceSurfacePresentModesKHR error");
        vkDestroySurfaceKHR(m_device->GetVkInstance(), m_vkSurface, nullptr);
        return false;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (uint32_t i = 0; i < unPresentModeCount; i++)
    {
        if (vPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }

        if (vPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;


    uint32_t unFormatCount = 0;
    result                 = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_vkSurface, &unFormatCount, NULL);
    if (result != VK_SUCCESS || unFormatCount < 1)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetPhysicalDeviceSurfaceFormatsKHR "
                                        "unFormatCount < 1");
        return false;
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(unFormatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_vkSurface, &unFormatCount, surfaceFormats.data());
    if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : "
                                        "GetPhysicalDeviceSurfaceFormatsKHR error");
        return false;
    }


    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_vkSurface, &vkSurfaceCapabilities);

    std::vector<VkCompositeAlphaFlagBitsKHR> vDefaultCompositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    };

    VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlag = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;

    for (auto flag: vDefaultCompositeAlphaFlags)
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

    VkFormat        colorFormat;
    VkColorSpaceKHR colorSpace;

    if (unFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    else
        colorFormat = surfaceFormats[0].format;
    colorSpace = surfaceFormats[0].colorSpace;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface                  = m_vkSurface;
    swapchainCreateInfo.minImageCount            = imageCount;
    swapchainCreateInfo.imageFormat              = colorFormat;
    swapchainCreateInfo.imageColorSpace          = colorSpace;
    swapchainCreateInfo.imageExtent              = {swapchainExtent.width, swapchainExtent.height};
    swapchainCreateInfo.imageArrayLayers         = 1;
    swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount    = 1;
    swapchainCreateInfo.pQueueFamilyIndices      = {0};
    swapchainCreateInfo.preTransform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha           = vkCompositeAlphaFlag;
    swapchainCreateInfo.presentMode              = presentMode;


    VkSwapchainKHR swapchain;
    result = vkCreateSwapchainKHR(m_device->m_vkDevice, &swapchainCreateInfo, NULL, &swapchain);
    if (result != VK_SUCCESS)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : CreateSwapchainKHR error");
        return false;
    }

    m_unMinImageCount = imageCount;
    m_vkSwapchain     = swapchain;
    m_vkColorFormat   = colorFormat;
    m_vkExtend        = swapchainExtent;

    BindRenderPass();
    return true;
}

void MRenderView::ReleaseSwapchain()
{
    if (VK_NULL_HANDLE != m_vkSwapchain)
    {
        vkDestroySwapchainKHR(m_device->m_vkDevice, m_vkSwapchain, nullptr);
        m_vkSwapchain = VK_NULL_HANDLE;
    }
}

bool MRenderView::BindRenderPass()
{
    DestroyRenderPass();

    uint32_t unSwapchainImageCount = 0;
    VkResult result = vkGetSwapchainImagesKHR(m_device->m_vkDevice, m_vkSwapchain, &unSwapchainImageCount, NULL);
    if (result != VK_SUCCESS || unSwapchainImageCount < 1)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : unSwapchainImageCount < 1");
        return false;
    }

    std::vector<VkImage> vSwapchainImages(unSwapchainImageCount);
    result = vkGetSwapchainImagesKHR(
            m_device->m_vkDevice,
            m_vkSwapchain,
            &unSwapchainImageCount,
            vSwapchainImages.data()
    );
    if (result != VK_SUCCESS || unSwapchainImageCount < 1)
    {
        GetEngine()->GetLogger()->Error("Create VulkanRenderTarget Error : GetSwapchainImagesKHR error");
        return false;
    }

    //index range is swapchain num

    Vector2i size(GetWidth(), GetHeight());
    m_renderTarget.resize(vSwapchainImages.size());
    for (size_t i = 0; i < vSwapchainImages.size(); ++i)
    {
        m_renderTarget[i].unImageIndex          = static_cast<uint32_t>(i);
        m_renderTarget[i].pPrimaryCommand       = nullptr;
        m_renderTarget[i].vkImageReadySemaphore = VK_NULL_HANDLE;

        auto pTexture   = MTexture::CreateTexture({
                  .strName         = "Editor Render View",
                  .n3Size          = Vector3i(size.x, size.y, 1),
                  .eTextureType    = METextureType::ETexture2DArray,
                  .eFormat         = METextureFormat::UNorm_RGBA8,
                  .eMipmapDataType = MEMipmapDataType::Disable,
                  .nReadUsage      = METextureReadUsageBit::EPixelSampler,
                  .nWriteUsage     = METextureWriteUsageBit::ERenderPresent,
        });
        auto textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();

        textureRHI->vkTextureImage       = vSwapchainImages[i];
        textureRHI->vkTextureImageMemory = VK_NULL_HANDLE;
        textureRHI->vkImageLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        textureRHI->vkTextureFormat      = m_vkColorFormat;

        pTexture->GenerateBuffer(m_device);
        m_renderTarget[i].renderPass.AddBackTexture(pTexture, {true, MColor::Black_T});

        MTexturePtr pDepthTexture =
                MTexture::CreateTexture(MTexture::CreateDepthBuffer("Editor Depth View").InitSize(size));

        pDepthTexture->GenerateBuffer(m_device);
        m_renderTarget[i].renderPass.SetDepthTexture(pDepthTexture, {true, MColor::White});
        m_renderTarget[i].renderPass.m_vkExtent2D = m_vkExtend;
        m_renderTarget[i].renderPass.GenerateBuffer(m_device);
    }


    return true;
}

void MRenderView::DestroyRenderPass()
{
    for (MViewRenderTarget rendertarget: m_renderTarget)
    {
        if (rendertarget.vkImageReadySemaphore)
        {
            m_device->GetRecycleBin()->DestroySemaphoreLater(rendertarget.vkImageReadySemaphore);
            rendertarget.vkImageReadySemaphore = VK_NULL_HANDLE;
        }

        for (MRenderTarget& tex: rendertarget.renderPass.m_renderTarget.backTargets)
        {
            tex.pTexture->DestroyBuffer(m_device);
            tex.pTexture = nullptr;
        }

        rendertarget.renderPass.m_renderTarget.backTargets.clear();

        if (MTexturePtr pDepthTexture = rendertarget.renderPass.GetDepthTexture())
        {
            pDepthTexture->DestroyBuffer(m_device);
            rendertarget.renderPass.SetDepthTexture(nullptr, {});
        }

        rendertarget.renderPass.DestroyBuffer(m_device);
        rendertarget.pPrimaryCommand = nullptr;
    }

    m_renderTarget.clear();
}

MViewRenderTarget* MRenderView::GetNextRenderTarget()
{
    if (VK_NULL_HANDLE == m_vkSwapchain) return nullptr;

    //get available image. by semaphore and index.
    uint32_t              unImageIndex          = 0;
    VkSemaphore           vkImageReadySemaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    MORTY_ASSERT(
            vkCreateSemaphore(m_device->m_vkDevice, &semaphoreInfo, nullptr, &vkImageReadySemaphore) == VK_SUCCESS
    );

    VkResult result = vkAcquireNextImageKHR(
            m_device->m_vkDevice,
            m_vkSwapchain,
            UINT64_MAX,
            vkImageReadySemaphore,
            VK_NULL_HANDLE,
            &unImageIndex
    );

    if (result != VK_SUCCESS)
    {
        return nullptr;// windows minimize or other, don`t render.
    }

    if (m_renderTarget[unImageIndex].vkImageReadySemaphore)
    {
        m_device->GetRecycleBin()->DestroySemaphoreLater(m_renderTarget[unImageIndex].vkImageReadySemaphore);
        m_renderTarget[unImageIndex].vkImageReadySemaphore = nullptr;
    }

    m_renderTarget[unImageIndex].vkImageReadySemaphore = vkImageReadySemaphore;

    //	GetEngine()->GetLogger()->Information("the Image View: {}", unImageIndex);

    //now, framebuffer is ready, commandbuffer is ready.
    return &m_renderTarget[unImageIndex];
}
