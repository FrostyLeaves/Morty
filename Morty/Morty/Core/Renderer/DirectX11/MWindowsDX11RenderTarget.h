/**
 * @File         MWindowsDX11RenderTarget
 * 
 * @Created      2019-12-30 11:43:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MWINDOWSDX11RENDERTARGET_H_
#define _M_MWINDOWSDX11RENDERTARGET_H_
#include "MGlobal.h"
#include <functional>

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include "MIRenderTarget.h"

class MIRenderer;
class MWindowsRenderView;
class MDirectX11Device;
class MORTY_CLASS MWindowsDX11RenderTarget : public MIRenderTarget
{
public:
	MWindowsDX11RenderTarget(MDirectX11Device* pDevice);
	~MWindowsDX11RenderTarget();

	virtual void OnResize(int nWidth, int nHeight) override;
	virtual void OnRender(MIRenderer* pRenderer) override;


	std::function<void(MIRenderer*)> m_funcRenderFunction;

	static MWindowsDX11RenderTarget* CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView);

private:
	MDirectX11Device* m_pDevice;
	IDXGISwapChain* m_pSwapChain;
};


#endif
