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

public:

	virtual void OnResize(const unsigned int& nWidth, const unsigned int& nHeight) = 0;

	virtual void OnRender(MIRenderer* pRenderer) { if(m_funcRenderFunction) m_funcRenderFunction(pRenderer); }
	std::function<void(MIRenderer*)> m_funcRenderFunction;

	virtual void Release(MIDevice* pDevice) = 0;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ID3D11RenderTargetView** m_vpTargetView;
	ID3D11DepthStencilView* m_pDepthStencilView;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

	MColor m_backgroundColor;

protected:

	unsigned int m_unTargetViewNum;
};


#endif
