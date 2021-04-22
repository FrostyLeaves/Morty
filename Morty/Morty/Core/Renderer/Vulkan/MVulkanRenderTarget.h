/**
 * @File         MVulkanRenderTarget
 * 
 * @Created      2020-06-17 21:01:03
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVULKANRENDERTARGET_H_
#define _M_MVULKANRENDERTARGET_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MIRenderTarget.h"
#include "MVulkanDevice.h"
#include "MRenderPass.h"

class MWindowsRenderView;
class MORTY_API MVulkanRenderTarget : public MIRenderTarget
{
public:
	M_OBJECT(MVulkanRenderTarget)
public:
    MVulkanRenderTarget();
    virtual ~MVulkanRenderTarget();

public:
	virtual void OnRender(MIRenderer* pRenderer) override;
	virtual void WaitImageReady() override;
	virtual void Present(MRenderCommand* pPrimaryCommand) override;

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void Initialize();

	virtual void Resize(const Vector2& v2Size) override;


	uint32_t GetFrameBufferIndex();
	MRenderCommand* GetPrimaryCommand();


	static MVulkanRenderTarget* CreateForSurface(MEngine* pEngine, MIRenderView* pView, VkSurfaceKHR surface);

#ifdef MORTY_WIN
	static MVulkanRenderTarget* CreateForWindowsView(MEngine* pEngine, MIRenderView* pView);
#endif

#ifdef MORTY_ANDROID
	static MVulkanRenderTarget* CreateForAndroidView(MEngine* pEngine, MIRenderView* pView);
#endif

public:

	bool InitializeSwapchain(MVulkanDevice* pDevice);

	void ReleaseSwapchain(MVulkanDevice* pDevice);


	void CleanRenderBuffer(MVulkanDevice* pDevice);
	bool RebindRenderBuffer(MVulkanDevice* pDevice);

public:

	MIRenderView* m_pView;
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	VkSemaphore m_VkImageAvailableSemaphore;

	uint32_t m_unFrameBufferIndex;
	uint32_t m_unMinImageCount;

	MRenderPass m_RenderPass;

	std::array<MRenderCommand*, M_BUFFER_NUM> m_aPrimaryCommands;

public:

};


#endif


#endif