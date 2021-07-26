/**
 * @File         MRenderView
 * 
 * @Created      2021-07-19 13:17:36
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERVIEW_H_
#define _M_MRENDERVIEW_H_
#include "MRenderGlobal.h"

#include "MEngine.h"
#include "MRenderPass.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanDevice.h"
#endif

class MTaskNode;
class MIRenderCommand;

class MORTY_API MRenderTarget
{
public:

	void BindPrimaryCommand(MIRenderCommand* pCommand) { pPrimaryCommand = pCommand; }

	MRenderPass renderPass;
	uint32_t unImageIndex;
	MIRenderCommand* pPrimaryCommand;

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


public:

	virtual void Initialize(MEngine* pEngine);
	virtual void Release();

	virtual void Render(MTaskNode* pTaskNode) {}

	MRenderTarget* GetNextRenderTarget();
	void Present(MRenderTarget* pRenderTarget);


#if RENDER_GRAPHICS == MORTY_VULKAN

	void InitializeForVulkan(MIDevice* pDevice, VkSurfaceKHR surface);
	bool InitializeSwapchain();
	void ReleaseSwapchain();

	bool BindRenderPass();

	void DestroyRenderPass();

	VkSemaphore GetSemaphore();
	VkSemaphore CreateSemaphore();

#endif

public:

	MEngine* GetEngine() { return m_pEngine; }

protected:

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	uint32_t m_unMinImageCount;

	VkFormat m_VkColorFormat;
	VkExtent2D m_VkExtend;

	MVulkanDevice* m_pDevice;
	std::queue<VkSemaphore> m_vImageReadySemaphore;
#endif

	std::vector<MRenderTarget> m_vRenderTarget;

private:

	MEngine* m_pEngine;

	uint32_t m_unWidht;
	uint32_t m_unHeight;
};


#endif
