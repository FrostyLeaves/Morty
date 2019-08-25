/**
 * @File         MDirectX11Renderer
 * 
 * @Created      2019-05-12 21:52:30
 *
 * @Author       Morty
**/

#ifndef _M_MDIRECTX11RENDERER_H_
#define _M_MDIRECTX11RENDERER_H_
#include "MGlobal.h"
#include "MIRenderer.h"
#include "MSingleInstance.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include <vector>

class MIRenderView;
class MVertexBuffer;
class MORTY_CLASS MDirectX11Renderer : public MIRenderer
	, public MSingleInstance<MDirectX11Renderer>
{
public:
    MDirectX11Renderer();
    virtual ~MDirectX11Renderer();

public:

	virtual void AddOutputView(MIRenderView* pView) override;
	virtual void RemoveOutputView(MIRenderView* pView) override;
	virtual void OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight) override;

	bool InitDirectX11();

	virtual bool Initialize() override;
	virtual void Release() override;
	virtual void RenderNodeToView(MNode* pNode, MIRenderView* pView) override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MMesh* pMesh) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;

	virtual void Draw(MVertexBuffer* pBuffer) override;

protected:

	struct RenderTarget
	{
		MIRenderView* pRenderView = nullptr;
		IDXGISwapChain* pSwapChain = nullptr;
		ID3D11RenderTargetView* pTargetView = nullptr;
		ID3D11Texture2D* pDepthStencilBuffer = nullptr;
		ID3D11DepthStencilView* pDepthStencilView = nullptr;

		D3D11_RASTERIZER_DESC mRasterizer;
	};

	RenderTarget CreateRenderTargetForWindow(MIRenderView* pView);
	void OnResize(RenderTarget& renderTarget, const int& nWidth, const int& nHeight);

protected:
	UINT m_n4xMsaaQuality;
	D3D_DRIVER_TYPE m_nDriverType;
	D3D_FEATURE_LEVEL m_nFeatureLevel;
	ID3D11Device* m_pD3dDevice;
	ID3D11DeviceContext* m_pD3dContext;

	ID3D11RasterizerState* m_pRasterizerState;
	
	std::vector<RenderTarget> m_vRenderTargets;
};


#endif
