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

	unsigned int GetTargetViewNum() const { return m_unTargetViewNum; }
	
	virtual MRenderDepthTexture* GetDepthTexture() = 0;

	virtual void SetBackgroundColor(const unsigned int& unTargetIndex, const MColor& color) = 0;
	virtual const MColor& GetBackgroundColor(const unsigned int& unTargetIndex) const { return MColor::Black; }

	virtual bool GetNeedCleanTargetView(const unsigned int& unTargetIndex) const { return true; }

public:

	virtual void OnResize(const unsigned int& nWidth, const unsigned int& nHeight) = 0;

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

	unsigned int m_unTargetViewNum;
};


#endif
