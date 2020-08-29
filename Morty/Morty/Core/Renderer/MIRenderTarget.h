/**
 * @File         MRenderTarget
 * 
 * @Created      2019-12-30 11:43:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIRENDERTARGET_H_
#define _M_MIRENDERTARGET_H_
#include "MGlobal.h"
#include "Type/MColor.h"
#include "MRenderPass.h"
#include "MIRenderer.h"

#include <array>
#include <vector>
#include <functional>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "vulkan/vulkan.h"
#endif

class MIDevice;
class MRenderDepthTexture;
class MIRenderBackTexture;

struct MORTY_CLASS MFrameBuffer
{
	MFrameBuffer();
	std::vector<MIRenderBackTexture*> vBackTextures;
	MRenderDepthTexture* pDepthTexture;
	VkFramebuffer vkFrameBuffer;
};

class MORTY_CLASS MIRenderTarget
{
public:

	MIRenderTarget();
	virtual ~MIRenderTarget() {}

	virtual MRenderDepthTexture* GetCurrDepthTexture() = 0;

	virtual uint32_t GetBackNum() = 0;
	virtual MColor GetBackClearColor(const uint32_t& unIndex) = 0;

	virtual bool GetDepthEnable() = 0;


	virtual uint32_t GetMFrameBufferNum() = 0;
	virtual MFrameBuffer* GetFrameBuffer(const uint32_t& unIndex) = 0;

	virtual MFrameBuffer* GetCurrFrameBuffer(const uint32_t& unFrameIdx = 0) = 0;
public:

	//virtual void OnResize(const uint32_t& nWidth, const uint32_t& nHeight) = 0;

	virtual void Render(MIRenderer* pRenderer) { pRenderer->Render(this); }

	virtual void OnRenderBefore(MIRenderer* pRenderer) {}
	virtual void OnRenderAfter(MIRenderer* pRenderer) {}
	virtual void OnRender(MIRenderer* pRenderer) { if(m_funcRenderFunction) m_funcRenderFunction(pRenderer); }
	std::function<void(MIRenderer*)> m_funcRenderFunction;

	virtual void Release(MIDevice* pDevice) = 0;

public:

	MRenderPass m_RenderPass;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() = 0;
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() = 0;
#elif RENDER_GRAPHICS == MORTY_VULKAN

	VkExtent2D m_VkExtend;
	VkFormat m_VkColorFormat;

	std::array<VkCommandBuffer, M_BUFFER_NUM> m_VkCommandBuffers;
	std::array<VkSemaphore, M_BUFFER_NUM> m_aVkRenderFinishedSemaphore;
	std::array<VkEvent, M_BUFFER_NUM> m_aVkRenderFinishedEvent;
#endif


};


#endif
