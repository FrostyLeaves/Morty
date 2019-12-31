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

class MIRenderer;
class MORTY_CLASS MIRenderTarget
{
public:
	MIRenderTarget();
	virtual ~MIRenderTarget() {}

	//×¼±¸äÖÈ¾×´Ì¬
	virtual void OnReadyRenderState() = 0;
	//»Ö¸´äÖÈ¾×´Ì¬
	virtual void OnRecoverRenderState() = 0;
	virtual void OnResize(int nWidth, int nHeight) = 0;

	virtual void OnRender(MIRenderer* pRenderer) { m_funcRenderFunction(pRenderer); }
	std::function<void(MIRenderer*)> m_funcRenderFunction;


#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ID3D11Texture2D* m_pBackBuffer;
	ID3D11RenderTargetView* m_pTargetView;
	ID3D11Texture2D* m_pDepthStencilBuffer;
	ID3D11DepthStencilView* m_pDepthStencilView;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif

};


#endif
