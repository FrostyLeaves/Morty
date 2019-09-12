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
	virtual void RenderNodeToView(MNode* pRootNode, MCamera* pNode, MIRenderView* pView) override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) override;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture) override;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) override;

	virtual void CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	virtual void DrawNode(MNode* pNode, const Matrix4& m4CameraInv) override;

	void DrawMesh(MIMesh* pMesh, const Matrix4& m4CameraInv, const Matrix4& m4ParentMat);

	virtual void SetUseMaterial(MMaterial* pMaterial) override;
	virtual void UpdateShaderParam(MShaderParam& param) override;

public:

	ID3D11Device* GetDevice(){ return m_pD3dDevice; }
	ID3D11DeviceContext* GetContext(){ return m_pD3dContext; }

	ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const int& nLength);

protected:

	struct RenderTarget
	{
		MIRenderView* pRenderView = nullptr;
		IDXGISwapChain* pSwapChain = nullptr;
		ID3D11RenderTargetView* pTargetView = nullptr;
		ID3D11Texture2D* pDepthStencilBuffer = nullptr;
		ID3D11DepthStencilView* pDepthStencilView = nullptr;
		D3D11_VIEWPORT mViewport;
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
	ID3D11InputLayout* m_pVertexInputLayout;

	ID3D11RasterizerState* m_pRasterizerState;
	
	std::vector<RenderTarget> m_vRenderTargets;
};


#endif
