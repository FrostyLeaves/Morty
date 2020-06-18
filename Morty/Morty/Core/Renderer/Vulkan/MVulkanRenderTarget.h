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

	virtual void SetBackgroundColor(const unsigned int& unTargetIndex, const MColor& color) override { m_backgroundColor = color; }
	virtual const MColor& GetBackgroundColor(const unsigned int& unTargetIndex) const override { return m_backgroundColor; }

	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

public:

	virtual void OnResize(const unsigned int& nWidth, const unsigned int& nHeight) override;
	virtual void OnRender(MIRenderer* pRenderer) override;

	virtual void Release(MIDevice* pDevice) override;

	static MVulkanRenderTarget* CreateForWindowsView(MVulkanDevice* pDevice, MWindowsRenderView* pView);

public:
	MColor m_backgroundColor;
	MRenderDepthTexture* m_pDepthTexture;

	MVulkanDevice* m_pDevice;
	VkSurfaceKHR m_VKSurface;
	VkSwapchainKHR m_VKSwapchain;
	VkFormat m_VKColorFormat;
	std::vector<VkImage> m_vSwapchainImages;

	
	//
public:
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImageView> m_vSwapchainImageView;
};


#endif


#endif