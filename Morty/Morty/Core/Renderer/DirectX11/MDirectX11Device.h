﻿/**
 * @File         MDirectX11Device
 * 
 * @Created      2019-09-21 23:12:50
 *
 * @Author       DoubleYe
**/


#ifndef _M_MDIRECTX11DEVICE_H_
#define _M_MDIRECTX11DEVICE_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#include "MIDevice.h"
#include "MVariant.h"

#include <d3d11.h>
#include <D3DX11.h>
#include <DxErr.h>


class MORTY_API MDirectX11Device : public MIDevice
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

	virtual void GenerateRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer, const METextureLayout& eType, const uint32_t& unWidth, const unsigned& unHeight) override;
	virtual void DestroyRenderTextureBuffer(MRenderTextureBuffer** ppTextureBuffer) override;

	virtual void GenerateDepthTexture(MDepthTextureBuffer** ppTextureBuffer, const uint32_t& unWidth, const uint32_t& unHeight) override;
	virtual void DestroyDepthTexture(MDepthTextureBuffer** ppTextureBuffer) override;

	virtual bool CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro) override;
	virtual void CleanShader(MShaderBuffer** ppShaderBuffer) override;

	ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const int& nLength);

	virtual bool GenerateRenderTarget(MIRenderTarget* pRenderTarget, uint32_t nWidth, uint32_t nHeight) override;
	virtual void DestroyRenderTarget(MIRenderTarget* pRenderTarget) override;

	virtual bool GenerateShaderParamBuffer(MShaderParam* pParam) override;
	virtual void DestroyShaderParamBuffer(MShaderParam* pParam) override;

	virtual bool GenerateRenderPass(MRenderPass* pRenderPass) override { return true; }
	virtual void DestroyRenderPass(MRenderPass* pRenderPass) override {}

	MVariant GenerateVariableByBuffer(class ID3D11ShaderReflectionType* pReflectionType);

	bool m_bEnable4xMsaa;

	ID3D11Device* m_pD3dDevice;
	ID3D11DeviceContext* m_pD3dContext;

	UINT m_n4xMsaaQuality;
	D3D_DRIVER_TYPE m_nDriverType;
	D3D_FEATURE_LEVEL m_nFeatureLevel;
};


#endif


#endif