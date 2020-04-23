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
#include <functional>

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include "MIRenderTarget.h"

class MIRenderer;
class MWindowsRenderView;
class MDirectX11Device;
class MORTY_CLASS MDirectX11RenderTarget : public MIRenderTarget
{
public:
	MDirectX11RenderTarget(MDirectX11Device* pDevice);
	~MDirectX11RenderTarget();

	virtual void OnResize(const unsigned int& nWidth, const unsigned int& nHeight) override;
	virtual void OnRender(MIRenderer* pRenderer) override;

	virtual void Release(MIDevice* pDevice) override;

	static MDirectX11RenderTarget* CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView);

public:
	IDXGISwapChain* m_pSwapChain;
private:
	MDirectX11Device* m_pDevice;
	MWindowsRenderView* m_pView;

};


#endif
