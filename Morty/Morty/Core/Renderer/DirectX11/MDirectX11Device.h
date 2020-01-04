/**
 * @File         MDirectX11Device
 * 
 * @Created      2019-09-21 23:12:50
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDIRECTX11DEVICE_H_
#define _M_MDIRECTX11DEVICE_H_
#include "MGlobal.h"
#include "MIDevice.h"
#include "MVariant.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>

class MORTY_CLASS MDirectX11Device : public MIDevice
{
public:
    MDirectX11Device();
    virtual ~MDirectX11Device();

public:
	bool InitDirectX11();
	virtual bool Initialize() override;
	virtual void Release() override;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) override;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) override;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) override;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap) override;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap) override;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) override;

	virtual void GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const unsigned int& unWidth, const unsigned int& unHeight) override;
	virtual void DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer) override;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const int& nLength);

	virtual bool GenerateRenderTarget(MIRenderTarget* pRenderTarget, unsigned int nWidth, unsigned int nHeight) override;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) override;

	virtual bool GenerateRenderTarget(MTextureRenderTarget* pRenderTarget, unsigned int nWidth, unsigned int nHeight) override;
	virtual void DestroyRenderTarget(MTextureRenderTarget* pRenderTarget) override;

	MVariant GenerateVariableByBuffer(class ID3D11ShaderReflectionType* pReflectionType);

	bool m_bEnable4xMsaa = true;

	ID3D11Device* m_pD3dDevice;
	ID3D11DeviceContext* m_pD3dContext;

	UINT m_n4xMsaaQuality;
	D3D_DRIVER_TYPE m_nDriverType;
	D3D_FEATURE_LEVEL m_nFeatureLevel;
};


#endif
