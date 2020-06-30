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
#include <functional>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "vulkan/vulkan.h"
#endif

#include "Type/MColor.h"

class MIDevice;
class MIRenderer;
class MRenderDepthTexture;

class MORTY_CLASS MIRenderTarget
{
public:


	MIRenderTarget();
	virtual ~MIRenderTarget() {}

	uint32_t GetTargetViewNum() const { return m_unTargetViewNum; }
	
	virtual MRenderDepthTexture* GetDepthTexture() = 0;

public:

	virtual void OnResize(const uint32_t& nWidth, const uint32_t& nHeight) = 0;

	virtual void OnRender(MIRenderer* pRenderer) { if(m_funcRenderFunction) m_funcRenderFunction(pRenderer); }
	std::function<void(MIRenderer*)> m_funcRenderFunction;

	virtual void Release(MIDevice* pDevice) = 0;

public:

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11RenderTargetView** m_vpRenderTargetView;
	struct ID3D11DepthStencilView* m_pDepthStencilView;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkRenderPass m_VkRenderPass;
#endif

protected:

	uint32_t m_unTargetViewNum;
};


#endif
