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
#include "MSingleInstance.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

#include <vector>

class MIRenderTarget;
class MIViewport;
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
	virtual void RemoveOutputView(MIRenderView* pView) override;

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void SetRenderTarget(MIRenderTarget* pRenderTarget) override { m_pRenderTarget = pRenderTarget; }
	virtual void SetViewport(MIViewport* pViewport) override;
	virtual void Render() override;

	virtual void InitDefaultResource() override;
	virtual void ReleaseDefaultResource() override;

public:
	virtual void DrawMesh(MIMesh* pMesh) override;

	virtual void SetUseMaterial(MMaterial* pMaterial) override;
	virtual void UpdateMaterialParam() override;
	virtual void UpdateMaterialResource() override;

public:
	void UpdateShaderParam(MShaderParam& param);
protected:

protected:
	ID3D11SamplerState* m_pDefaultSamplerState;
//	ID3D11SamplerState* m_pAnisotropicFilterSamplerState;

	ID3D11DepthStencilState* m_pDepthStencilState;

	ID3D11RasterizerState* m_pRasterizerState_Wireframe_CullNone;
	ID3D11RasterizerState* m_pRasterizerState_Solid_CullNone;
	ID3D11RasterizerState* m_pRasterizerState_Solid_CullBack;
	
	MDirectX11Device* m_pDevice;
	MMaterial* m_pUsingMaterial;
	MIRenderTarget* m_pRenderTarget;
};


#endif
