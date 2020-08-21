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

	virtual uint32_t GetBackNum() { return 1; }
	//unIndex is frameIndex
	virtual MRenderTextureBuffer** GetBackBuffers(const uint32_t& unIndex) override{ return m_vBackBuffers.empty() ? nullptr : &m_vBackBuffers[unIndex];}
	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }
	virtual VkFramebuffer GetFrameBuffer(const uint32_t& unIndex) override;

	virtual MColor GetBackClearColor(const uint32_t& unIndex) override;;

public:
	virtual void Render(MIRenderer* pRenderer) override;
	virtual void OnRender(MIRenderer* pRenderer) override;
	virtual void OnRenderAfter(MIRenderer* pRenderer) override;
	void Initialize();
	virtual void Release(MIDevice* pDevice) override;

	void Resize(const uint32_t& nWidth, const uint32_t& nHeight);

	static MVulkanRenderTarget* CreateForWindowsView(MVulkanDevice* pDevice, MWindowsRenderView* pView);


public:

	bool InitializeSwapchain();

	void ReleaseSwapchain();

	bool RebindRenderBuffer();

public:

	MIRenderView* m_pView;
	MVulkanDevice* m_pDevice;
	VkSurfaceKHR m_VkSurface;
	VkSwapchainKHR m_VkSwapchain;

	VkQueue m_VkPresentQueue;

	//没有MutilRenderTarget

	//size is swapchain size.
	std::vector<MRenderTextureBuffer*> m_vBackBuffers;
	std::vector<VkFramebuffer> m_VkFrameBuffer;

	MRenderDepthTexture* m_pDepthTexture;//swapchain的size和对不上的，所以没法把它作为Shader的Texture参数

	uint32_t m_unFrameBufferIndex;

public:

};


#endif


#endif