/**
 * @File         MDirectX11Renderer
 * 
 * @Created      2019-05-12 21:52:30
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDIRECTX11RENDERER_H_
#define _M_MDIRECTX11RENDERER_H_
#include "MGlobal.h"
#include "MIRenderer.h"
#include "MRenderStructure.h"
#include "MSingleInstance.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include <stack>

class MIRenderTarget;
class MViewport;
class MIRenderView;
class MVertexBuffer;
class MDirectX11Device;
class MORTY_CLASS MDirectX11Renderer : public MIRenderer
{
public:
    MDirectX11Renderer(MDirectX11Device* pDevice);
    virtual ~MDirectX11Renderer();

public:

public:

	virtual void AddOutputView(MIRenderView* pView) override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) override;
	virtual void Render(MIRenderTarget* pRenderTarget) override;
	virtual void RecoverRenderTarget(MIRenderTarget* pRenderTarget) override;

public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual bool SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources = false) override;
	virtual void UpdateMaterialParam() override;
	virtual void UpdateMaterialResource() override;

public:
	void UpdateShaderParam(MShaderParam& param);

	virtual void SetVertexShaderParam(MShaderParam& param) override;
	virtual void SetPixelShaderParam(MShaderParam& param) override;

	virtual void SetVertexShaderTexture(MShaderTextureParam& param) override;
	virtual void SetPixelShaderTexture(MShaderTextureParam& param) override;

protected:

protected:
	ID3D11SamplerState* m_pDefaultSamplerState;
	ID3D11SamplerState* m_pDepthTextureSamplerState;
//	ID3D11SamplerState* m_pAnisotropicFilterSamplerState;

	std::vector<ID3D11RasterizerState*> m_vRasterizerState;
	std::vector<ID3D11BlendState*> m_vBlendState;
	std::vector<ID3D11DepthStencilState*> m_vDepthStencilState;
	
	MDirectX11Device* m_pDevice;
	MMaterial* m_pUsingMaterial;
	//MIRenderTarget* m_pCurrentRenderTarget;

	std::stack<MIRenderTarget*> m_vRenderTargets;
};


#endif
