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

	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

public:

	virtual void OnRender(MIRenderer* pRenderer) override;

	virtual void Release(MIDevice* pDevice) override;

	void Resize(const uint32_t& nWidth, const uint32_t& nHeight);

	static MVulkanRenderTarget* CreateForWindowsView(MVulkanDevice* pDevice, MWindowsRenderView* pView);

	virtual VkFramebuffer GetFrameBuffer(const uint32_t& unIndex) override { return swapChainFramebuffers[unIndex]; }

public:

	MIRenderView* m_pView;
	MVulkanDevice* m_pDevice;
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	std::vector<VkImage> m_vSwapchainImages;
	std::vector<MRenderTargetView> m_RenderTargetView;
	MRenderDepthTexture* m_pDepthTexture;

	
	//
public:
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImageView> m_vSwapchainImageView;
};


#endif


#endif