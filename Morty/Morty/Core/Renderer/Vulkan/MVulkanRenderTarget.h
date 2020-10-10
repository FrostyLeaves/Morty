/**
 * @File         MVulkanRenderTarget
 * 
 * @Created      2020-06-17 21:01:03
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANRENDERTARGET_H_
#define _M_MVULKANRENDERTARGET_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MIRenderTarget.h"
#include "MVulkanDevice.h"

class MWindowsRenderView;
class MRenderDepthTexture;
class MORTY_CLASS MVulkanRenderTarget : public MIRenderTarget
{
public:
	M_OBJECT(MVulkanRenderTarget)
public:
    MVulkanRenderTarget();
    virtual ~MVulkanRenderTarget();

	virtual MRenderDepthTexture* GetCurrDepthTexture() override;

	virtual bool GetDepthEnable() override { return true; }

	virtual uint32_t GetMFrameBufferNum() override { return m_vBufferInfo.size(); }
	virtual MFrameBuffer* GetFrameBuffer(const uint32_t& unIndex) override;
	virtual MFrameBuffer* GetCurrFrameBuffer(const uint32_t& unFrameIdx = 0) override;

public:
	virtual void OnRender(MIRenderer* pRenderer) override;
	virtual void OnRenderBefore(MIRenderer* pRenderer) override;
	virtual void OnRenderAfter(MIRenderer* pRenderer) override;
	void Initialize();
	virtual void Release() override;

	virtual void Resize(const Vector2& v2Size) override;

#ifdef MORTY_WIN
	static MVulkanRenderTarget* CreateForWindowsView(MEngine* pEngine, MIRenderView* pView);
#endif

#ifdef MORTY_ANDROID
	static MVulkanRenderTarget* CreateForAndroidView(MEngine* pEngine, MIRenderView* pView);
#endif

public:

	bool InitializeSwapchain(MVulkanDevice* pDevice);

	void ReleaseSwapchain(MVulkanDevice* pDevice);

	bool RebindRenderBuffer(MVulkanDevice* pDevice);

	void CleanRenderInfo(MVulkanDevice* pDevice);

public:

	MIRenderView* m_pView;
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	VkSemaphore m_VkImageAvailableSemaphore;


	//size is swapchain size.
	std::vector<MFrameBuffer> m_vBufferInfo;

	uint32_t m_unFrameBufferIndex;
	uint32_t m_unMinImageCount;

public:

};


#endif


#endif