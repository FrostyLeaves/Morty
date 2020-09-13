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
#include "MWindowsRenderView.h"

class MRenderDepthTexture;
class MORTY_CLASS MVulkanRenderTarget : public MIRenderTarget
{
public:
    MVulkanRenderTarget(MVulkanDevice* pDevice);
    virtual ~MVulkanRenderTarget();

	virtual MRenderDepthTexture* GetCurrDepthTexture() override;

	virtual uint32_t GetBackNum() override { return 1; }
	virtual MColor GetBackClearColor(const uint32_t& unIndex) override;

	virtual bool GetDepthEnable() override { return true; }

	virtual uint32_t GetMFrameBufferNum() override { return m_vBufferInfo.size(); }
	virtual MFrameBuffer* GetFrameBuffer(const uint32_t& unIndex) override;
	virtual MFrameBuffer* GetCurrFrameBuffer(const uint32_t& unFrameIdx = 0) override;

public:
	virtual void OnRender(MIRenderer* pRenderer) override;
	virtual void OnRenderBefore(MIRenderer* pRenderer) override;
	virtual void OnRenderAfter(MIRenderer* pRenderer) override;
	void Initialize();
	virtual void Release(MIDevice* pDevice) override;

	void Resize(const uint32_t& nWidth, const uint32_t& nHeight);

	static MVulkanRenderTarget* CreateForWindowsView(MVulkanDevice* pDevice, MWindowsRenderView* pView);

public:

	bool InitializeSwapchain();

	void ReleaseSwapchain();

	bool RebindRenderBuffer();

	void CleanRenderInfo();

public:

	MIRenderView* m_pView;
	MVulkanDevice* m_pDevice;
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	VkSemaphore m_VkImageAvailableSemaphore;


	//size is swapchain size.
	std::vector<MFrameBuffer> m_vBufferInfo;

	uint32_t m_unFrameBufferIndex;
	uint32_t m_unMinImageCount;

	Vector2 m_v2Size;

public:

};


#endif


#endif