/**
 * @File         MRenderView
 * 
 * @Created      2021-07-19 13:17:36
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

#include "Engine/MEngine.h"
#include "RHI/MRenderPass.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanDevice.h"

#endif

namespace morty
{

class MTaskNode;
class MIRenderCommand;
class MORTY_API MViewRenderTarget
{
public:
    void                         BindPrimaryCommand(MIRenderCommand* pCommand);

    MRenderPass                  renderPass;
    uint32_t                     unImageIndex    = 0;
    MVulkanPrimaryRenderCommand* pPrimaryCommand = nullptr;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkSemaphore vkImageReadySemaphore;
#endif
};

class MORTY_API MRenderView
{
public:
    MRenderView();

    virtual ~MRenderView();

public:
    uint32_t           GetWidth() { return m_widht; }

    uint32_t           GetHeight() { return m_height; }

    void               InitSize(uint32_t nWidht, uint32_t nHeight);

    void               Resize(const Vector2& v2Size);

    virtual void       Initialize(MEngine* pEngine);

    virtual void       Release();

    virtual void       Render(MTaskNode* pTaskNode) = 0;

    MViewRenderTarget* GetNextRenderTarget();

    void               Present(MViewRenderTarget* pRenderTarget);


#if RENDER_GRAPHICS == MORTY_VULKAN

    void InitializeForVulkan(MIDevice* pDevice, VkSurfaceKHR surface);

    bool InitializeSwapchain();

    void ReleaseSwapchain();

    bool BindRenderPass();

    void DestroyRenderPass();

#endif

    MEngine* GetEngine() { return m_engine; }

    size_t   GetImageCount() { return m_renderTarget.size(); }

protected:
    void PresetWork(MViewRenderTarget* pRenderTarget);


#if RENDER_GRAPHICS == MORTY_VULKAN
    VkSurfaceKHR   m_vkSurface;
    VkSwapchainKHR m_vkSwapchain;

    uint32_t       m_unMinImageCount;

    VkFormat       m_vkColorFormat;
    VkExtent2D     m_vkExtend;

    MVulkanDevice* m_device;

#endif

    std::vector<MViewRenderTarget> m_renderTarget;

private:
    std::atomic<bool> m_submitFinished = true;

    MEngine*          m_engine = nullptr;

    uint32_t          m_widht  = 800;
    uint32_t          m_height = 600;
};

}// namespace morty