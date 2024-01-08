/**
 * @File         MRenderView
 * 
 * @Created      2021-07-19 13:17:36
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#include "Engine/MEngine.h"
#include "Render/MRenderPass.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanDevice.h"
#endif

class MTaskNode;
class MIRenderCommand;

class MORTY_API MViewRenderTarget
{
public:

	void BindPrimaryCommand(MIRenderCommand* pCommand);

	MRenderPass renderPass;
	uint32_t unImageIndex = 0;
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

	uint32_t GetWidth() { return m_unWidht; }
	uint32_t GetHeight() { return m_unHeight; }

	void Resize(const Vector2& v2Size);

	virtual void Initialize(MEngine* pEngine);
	virtual void Release();

	virtual void Render(MTaskNode* pTaskNode) = 0;

	MViewRenderTarget* GetNextRenderTarget();
	void Present(MViewRenderTarget* pRenderTarget);


#if RENDER_GRAPHICS == MORTY_VULKAN

	void InitializeForVulkan(MIDevice* pDevice, VkSurfaceKHR surface);
	bool InitializeSwapchain();
	void ReleaseSwapchain();

	bool BindRenderPass();

	void DestroyRenderPass();
	
#endif

	MEngine* GetEngine() { return m_pEngine; }

	size_t GetImageCount() { return m_vRenderTarget.size(); }

protected:


	void PresetWork(MViewRenderTarget* pRenderTarget);


#if RENDER_GRAPHICS == MORTY_VULKAN
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	uint32_t m_unMinImageCount;

	VkFormat m_VkColorFormat;
	VkExtent2D m_VkExtend;

	MVulkanDevice* m_pDevice;

#endif

	std::vector<MViewRenderTarget> m_vRenderTarget;

private:

	std::atomic<bool> m_bSubmitFinished = true;

	MEngine* m_pEngine = nullptr;

	uint32_t m_unWidht = 800;
	uint32_t m_unHeight = 600;
};
