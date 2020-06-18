/**
 * @File         MDirectX11RenderTarget
 * 
 * @Created      2019-12-30 11:43:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDIRECTX11RENDERTARGET_H_
#define _M_MDIRECTX11RENDERTARGET_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#include <functional>

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include "MIRenderTarget.h"

class MIRenderer;
class MWindowsRenderView;
class MDirectX11Device;
class MRenderDepthTexture;
class MORTY_CLASS MDirectX11RenderTarget : public MIRenderTarget
{
public:
	MDirectX11RenderTarget(MDirectX11Device* pDevice);
	~MDirectX11RenderTarget();

	virtual void SetBackgroundColor(const unsigned int& unTargetIndex, const MColor& color) override { m_backgroundColor = color; }
	virtual const MColor& GetBackgroundColor(const unsigned int& unTargetIndex) const override { return m_backgroundColor; }

	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

public:

	virtual void OnResize(const unsigned int& nWidth, const unsigned int& nHeight) override;
	virtual void OnRender(MIRenderer* pRenderer) override;

	virtual void Release(MIDevice* pDevice) override;

	static MDirectX11RenderTarget* CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView);

public:
	IDXGISwapChain* m_pSwapChain;
	MRenderDepthTexture* m_pDepthTexture;
private:
	MColor m_backgroundColor;
	MDirectX11Device* m_pDevice;
	MWindowsRenderView* m_pView;

};


#endif


#endif