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
#include "MRenderStructure.h"

class MIRenderer;
class MWindowsRenderView;
class MDirectX11Device;
class MRenderDepthTexture;
class MORTY_CLASS MDirectX11RenderTarget : public MIRenderTarget
{
public:
	MDirectX11RenderTarget(MDirectX11Device* pDevice);
	~MDirectX11RenderTarget();

	virtual MRenderDepthTexture* GetDepthTexture() override { return m_pDepthTexture; }

public:

	void Resize(const uint32_t& nWidth, const uint32_t& nHeight);
	virtual void OnRender(MIRenderer* pRenderer) override;

	virtual void Release(MIDevice* pDevice) override;

	static MDirectX11RenderTarget* CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView);

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	virtual std::vector<struct ID3D11RenderTargetView*> GetRenderTargetViews() override { return { m_RenderTargetView.m_pRenderTargetView }; }
	virtual struct ID3D11DepthStencilView* GetDepthStencilView() override;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	VkRenderPass m_VkRenderPass;
#endif

public:
	IDXGISwapChain* m_pSwapChain;
	MRenderTargetView m_RenderTargetView;
	MRenderDepthTexture* m_pDepthTexture;
private:
	MDirectX11Device* m_pDevice;
	MWindowsRenderView* m_pView;

};


#endif


#endif