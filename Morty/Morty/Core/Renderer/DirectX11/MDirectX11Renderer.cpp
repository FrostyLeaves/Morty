#include "MDirectX11Renderer.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#include "MWindowsRenderView.h"
#include "MLogManager.h"

#include "D3Dcompiler.h"
#include "d3d11shader.h"

#include "MDirectX11Device.h"
#include "MDirectX11RenderTarget.h"

#include "MTexture.h"
#include "MShader.h"
#include "MMaterial.h"
#include "MMesh.h"
#include "MViewport.h"
#include "MIRenderTarget.h"
#include "MRenderStructure.h"

#if MORTY_RENDER_DATA_STATISTICS
#include "MRenderStatistics.h"
#endif

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

const bool bEnable4xMsaa = true;

MDirectX11Renderer::MDirectX11Renderer(MDirectX11Device* pDevice)
	: m_pDevice(pDevice)
	, m_pDefaultSamplerState(nullptr)
	, m_pLessEqualSamplerState(nullptr)
	, m_pGreaterEqualSamplerState(nullptr)
	, m_vDepthStencilState()
	, m_vRasterizerState()
	, m_vBlendState()
	, m_pUsingMaterial(nullptr)
//	, m_pCurrentRenderTarget(nullptr)
{

}

MDirectX11Renderer::~MDirectX11Renderer()
{

}

void MDirectX11Renderer::AddOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{
		MDirectX11RenderTarget::CreateForView(m_pDevice, pWindowView);
	}
}

bool MDirectX11Renderer::Initialize()
{

	//光栅化状态块
	D3D11_RASTERIZER_DESC mRasterizer;
	ZeroMemory(&mRasterizer, sizeof(D3D11_RASTERIZER_DESC));
	//实心模式，WIREFRAME是线框模式
	mRasterizer.FillMode = D3D11_FILL_SOLID;
	mRasterizer.CullMode = D3D11_CULL_BACK;
	mRasterizer.FrontCounterClockwise = false;
	mRasterizer.DepthClipEnable = true;//这个裁剪指的是平截头裁剪；

	if(m_pDevice->m_bEnable4xMsaa)
		mRasterizer.MultisampleEnable = true;

	m_vRasterizerState.resize((int)MERasterizerType::ERasterizerEnd);
	HRESULT hr = m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_vRasterizerState[(int)MERasterizerType::ECullBack]);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create RasterizerState! Initialize return false.");
		if (m_vRasterizerState[(int)MERasterizerType::ECullBack])
		{
			m_vRasterizerState[(int)MERasterizerType::ECullBack]->Release();
			m_vRasterizerState[(int)MERasterizerType::ECullBack] = nullptr;
		}
		return false;
	}

	mRasterizer.CullMode = D3D11_CULL_FRONT;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_vRasterizerState[(int)MERasterizerType::ECullFront]);
	mRasterizer.CullMode = D3D11_CULL_NONE;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_vRasterizerState[(int)MERasterizerType::ECullNone]);
	mRasterizer.FillMode = D3D11_FILL_WIREFRAME;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_vRasterizerState[(int)MERasterizerType::EWireframe]);



	m_vBlendState.resize((int)MEMaterialType::EMaterialTypeEnd);
	//混合状态块
	D3D11_BLEND_DESC mBlendDesc;
	mBlendDesc.AlphaToCoverageEnable = false;
	mBlendDesc.IndependentBlendEnable = false;

	D3D11_RENDER_TARGET_BLEND_DESC& mRTDesc = mBlendDesc.RenderTarget[0];
	mRTDesc.BlendEnable = true;
	mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	mRTDesc.DestBlend = D3D11_BLEND_ZERO;
	mRTDesc.DestBlendAlpha = D3D11_BLEND_ZERO;

	mRTDesc.SrcBlend = D3D11_BLEND_ONE;
	mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;

	mRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
	mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;


	m_pDevice->m_pD3dDevice->CreateBlendState(&mBlendDesc, &m_vBlendState[(int)MEBlendType::EDefault]);
	
	mRTDesc.BlendEnable = true;
	mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	mRTDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	mRTDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	
	mRTDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;

	mRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
	mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;

	m_pDevice->m_pD3dDevice->CreateBlendState(&mBlendDesc, &m_vBlendState[(int)MEBlendType::ETransparent]);


	{
		//混合状态块
		D3D11_BLEND_DESC mBlendDesc;
		mBlendDesc.AlphaToCoverageEnable = false;
		mBlendDesc.IndependentBlendEnable = true;

		//Front
		{
			D3D11_RENDER_TARGET_BLEND_DESC& mRTDesc = mBlendDesc.RenderTarget[0];
			mRTDesc.BlendEnable = true;
 			mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			mRTDesc.DestBlend = D3D11_BLEND_ONE;
			mRTDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.SrcBlend = D3D11_BLEND_INV_DEST_ALPHA;
			mRTDesc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
			mRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
			mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		}
		//Back
		{
			D3D11_RENDER_TARGET_BLEND_DESC& mRTDesc = mBlendDesc.RenderTarget[1];
			mRTDesc.BlendEnable = true;
			mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			mRTDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			mRTDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			mRTDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
			mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
// 			mRTDesc.DestBlend = D3D11_BLEND_ONE;
// 			mRTDesc.DestBlendAlpha = D3D11_BLEND_ONE;
// 
// 			mRTDesc.SrcBlend = D3D11_BLEND_ONE;
// 			mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
// 
// 			mRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
// 			mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		}
		{//Front Depth
			D3D11_RENDER_TARGET_BLEND_DESC& mRTDesc = mBlendDesc.RenderTarget[2];
			mRTDesc.BlendEnable = true;
			mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			mRTDesc.DestBlend = D3D11_BLEND_ONE;
			mRTDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.SrcBlend = D3D11_BLEND_ONE;
			mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.BlendOp = D3D11_BLEND_OP_MIN;
			mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_MIN;
		}
		for(uint32_t i = 3; i < 8 ; ++i)
		{
			//Back Depth
			D3D11_RENDER_TARGET_BLEND_DESC& mRTDesc = mBlendDesc.RenderTarget[i];
			mRTDesc.BlendEnable = true;
			mRTDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			mRTDesc.DestBlend = D3D11_BLEND_ONE;
			mRTDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.SrcBlend = D3D11_BLEND_ONE;
			mRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			mRTDesc.BlendOp = D3D11_BLEND_OP_MAX;
			mRTDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;
		}

		m_pDevice->m_pD3dDevice->CreateBlendState(&mBlendDesc, &m_vBlendState[(int)MEBlendType::EAlphaOverlying]);
	}

	m_pDevice->m_pD3dContext->OMSetBlendState(m_vBlendState[(int)MEBlendType::EDefault], nullptr, 0xffffffff);

	//创建纹理采样状态
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_pDevice->m_pD3dDevice->CreateSamplerState(&sampDesc, &m_pDefaultSamplerState);

// 	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
// 	hr = m_pDevice->m_pD3dDevice->CreateSamplerState(&sampDesc, &m_pAnisotropicFilterSamplerState);


	m_vDepthStencilState.resize((int)MEDepthStencilType::EDepthTypeEnd);
	//深度状态
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;

	m_pDevice->m_pD3dDevice->CreateDepthStencilState(&dsDesc, &m_vDepthStencilState[(int)MEDepthStencilType::EDefault]);
	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EDefault], 0);

	dsDesc.DepthEnable = true;
	dsDesc.StencilEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	m_pDevice->m_pD3dDevice->CreateDepthStencilState(&dsDesc, &m_vDepthStencilState[(int)MEDepthStencilType::EReadNotWrite]);

	dsDesc.DepthEnable = false;
	dsDesc.StencilEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	m_pDevice->m_pD3dDevice->CreateDepthStencilState(&dsDesc, &m_vDepthStencilState[(int)MEDepthStencilType::ENotReadNotWrite]);

	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	m_pDevice->m_pD3dDevice->CreateSamplerState(&comparisonSamplerDesc, &m_pLessEqualSamplerState);


	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	m_pDevice->m_pD3dDevice->CreateSamplerState(&comparisonSamplerDesc, &m_pGreaterEqualSamplerState);

	

	//三角形解析顶点
	m_pDevice->m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pDevice->m_pD3dContext->VSSetSamplers(0, 0, &m_pDefaultSamplerState);
	m_pDevice->m_pD3dContext->VSSetSamplers(1, 1, &m_pLessEqualSamplerState);
	m_pDevice->m_pD3dContext->VSSetSamplers(2, 1, &m_pGreaterEqualSamplerState);
	m_pDevice->m_pD3dContext->PSSetSamplers(0, 0, &m_pDefaultSamplerState);
	m_pDevice->m_pD3dContext->PSSetSamplers(1, 1, &m_pLessEqualSamplerState);
	m_pDevice->m_pD3dContext->PSSetSamplers(2, 1, &m_pGreaterEqualSamplerState);


	return true;
}

void MDirectX11Renderer::Release()
{
	for (ID3D11RasterizerState* pRasterizerState : m_vRasterizerState)
	{
		if (pRasterizerState)
		{
			pRasterizerState->Release();
			pRasterizerState = nullptr;
		}
	}
	for (ID3D11BlendState* pBlendState : m_vBlendState)
	{
		if (pBlendState)
		{
			pBlendState->Release();
			pBlendState = nullptr;
		}
	}
	for (ID3D11DepthStencilState* pDepthStencilState : m_vDepthStencilState)
	{
		if (pDepthStencilState)
		{
			pDepthStencilState->Release();
			pDepthStencilState = nullptr;
		}
	}
	if (m_pDefaultSamplerState)
	{
		m_pDefaultSamplerState->Release();
		m_pDefaultSamplerState = nullptr;
	}
	if (m_pLessEqualSamplerState)
	{
		m_pLessEqualSamplerState->Release();
		m_pLessEqualSamplerState = nullptr;
	}
	if (m_pGreaterEqualSamplerState)
	{
		m_pGreaterEqualSamplerState->Release();
		m_pGreaterEqualSamplerState = nullptr;
	}
}

void MDirectX11Renderer::SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth)
{
	static D3D11_VIEWPORT viewport;
	viewport.Width = fWidth;
	viewport.Height = fHeight;
	viewport.MinDepth = fMinDepth;
	viewport.MaxDepth = fMaxDepth;
	viewport.TopLeftX = fX;
	viewport.TopLeftY = fY;

	m_pDevice->m_pD3dContext->RSSetViewports(1, &viewport);
}

void MDirectX11Renderer::RecoverRenderTarget(RenderTargetPair& rtp)
{
	//warning! Material may has been switched.


	//warning, this could be changed by outer.so can`t do this.
	//if (m_pCurrentRenderTarget != pRenderTarget)
	{
		m_pUsingMaterial = nullptr;

		uint32_t unTargetSize = rtp.pRenderTarget->GetTargetViewNum();
		ID3D11DepthStencilView* pDepthStencilView = nullptr;
		if (rtp.pDepthTexture->GetDepthBuffer())
			pDepthStencilView = rtp.pDepthTexture->GetDepthBuffer()->m_pDepthStencilView;

		m_pDevice->m_pD3dContext->OMSetRenderTargets(unTargetSize, rtp.pRenderTarget->m_vpRenderTargetView, pDepthStencilView);
	}
}

void MDirectX11Renderer::ClearRenderTargetView(MIRenderTarget* pRenderTarget, const uint32_t& unViewIndex, const MColor& color)
{
	if (pRenderTarget)
	{
		m_pDevice->m_pD3dContext->ClearRenderTargetView(pRenderTarget->m_vpRenderTargetView[unViewIndex], color.m);
	}
}

void MDirectX11Renderer::ClearDepthTexture(MRenderDepthTexture* pDepthTexture)
{
	if (MDepthTextureBuffer* pBuffer = pDepthTexture->GetDepthBuffer())
	{
		if (pBuffer->m_pDepthStencilView)
		{
			m_pDevice->m_pD3dContext->ClearDepthStencilView(pBuffer->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
	}
}

bool MDirectX11Renderer::SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources/* = false*/)
{
	if (m_pUsingMaterial == pMaterial)
		return true;

	if (nullptr == pMaterial)
	{
		//TODO 使用默认材质
		return false;
	}

	MShader* pVertexShader = pMaterial->GetVertexShader();
	MShader* pPixelShader = pMaterial->GetPixelShader();

	if (nullptr == pVertexShader || nullptr == pPixelShader)
	{
		return false;
	}

	m_pUsingMaterial = pMaterial;

	if (nullptr == pVertexShader->GetBuffer())
		return false;
	if (nullptr == pPixelShader->GetBuffer())
		return false;

	if (MVertexShaderBuffer* pVertexShaderBuffer = dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer()))
	{
		if (pVertexShaderBuffer->m_pInputLayout)
		{
			m_pDevice->m_pD3dContext->IASetInputLayout(pVertexShaderBuffer->m_pInputLayout);
		}
	}

	m_pDevice->m_pD3dContext->VSSetShader(dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer())->m_pVertexShader, nullptr, 0);
	m_pDevice->m_pD3dContext->PSSetShader(dynamic_cast<MPixelShaderBuffer*>(pPixelShader->GetBuffer())->m_pPixelShader, nullptr, 0);

	
	if (m_eRasterizerType != pMaterial->GetRasterizerType())
	{
		//切换渲染状态
		m_eRasterizerType = pMaterial->GetRasterizerType();
		m_pDevice->m_pD3dContext->RSSetState(m_vRasterizerState[(int)m_eRasterizerType]);
	}

	if (m_eMaterialType != pMaterial->GetMaterialType())
	{
		m_eMaterialType = pMaterial->GetMaterialType();

		if (MEMaterialType::EDefault == m_eMaterialType)
		{
			m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EDefault], 0);
			m_pDevice->m_pD3dContext->OMSetBlendState(m_vBlendState[(int)MEBlendType::EDefault], nullptr, 0xffffffff);
		}
		else if (MEMaterialType::ETransparent == m_eMaterialType)
		{
			m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EReadNotWrite], 0);
			m_pDevice->m_pD3dContext->OMSetBlendState(m_vBlendState[(int)MEBlendType::EAlphaOverlying], nullptr, 0xffffffff);
		}
		else if (MEMaterialType::EBlendTransparent == m_eMaterialType)
		{
			m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::ENotReadNotWrite], 0);
			m_pDevice->m_pD3dContext->OMSetBlendState(m_vBlendState[(int)MEBlendType::ETransparent], nullptr, 0xffffffff);
		}
		
	}


	//更新材质使用的资源
	if (bUpdateResources)
	{
		UpdateMaterialResource();
		UpdateMaterialParam();

	}

	return true;
}

void MDirectX11Renderer::DrawMesh(MIMesh* pMesh)
{
	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(m_pDevice);

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(m_pDevice);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		UINT stride = pMesh->GetVertexStructSize();
		UINT offset = 0;
		m_pDevice->m_pD3dContext->IASetVertexBuffers(0, 1, &pBuffer->m_pVertexBuffer, &stride, &offset);

		m_pDevice->m_pD3dContext->IASetIndexBuffer(pBuffer->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		m_pDevice->m_pD3dContext->DrawIndexed(pMesh->GetIndicesLength(), 0, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}
}

void MDirectX11Renderer::UpdateMaterialParam()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderParam& param : *m_pUsingMaterial->GetShaderParams())
	{
		SetShaderParam(param);
	}

}

void MDirectX11Renderer::UpdateMaterialResource()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderTextureParam& param : *m_pUsingMaterial->GetTextureParams())
	{
		if ((int)MEShaderParamType::EVertex & param.eShaderType)
			SetVertexShaderTexture(param);
		if ((int)MEShaderParamType::EPixel & param.eShaderType)
			SetPixelShaderTexture(param);
	}
}

void MDirectX11Renderer::UpdateShaderParam(MShaderParam& param)
{
	if (nullptr == param.pBuffer)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//  Disable GPU access to the vertex buffer data.
	m_pDevice->m_pD3dContext->Map(param.pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//  Update the vertex buffer here.

	memcpy(mappedResource.pData, param.var.GetData(), param.var.GetSize());
	//  Reenable GPU access to the vertex buffer data.
	m_pDevice->m_pD3dContext->Unmap(param.pBuffer, 0);

	param.bDirty = false;
}

void MDirectX11Renderer::SetShaderParam(MShaderParam& param)
{
	if (param.bDirty)
		UpdateShaderParam(param);

	if(param.eType & MShader::MEShaderType::Vertex)
		m_pDevice->m_pD3dContext->VSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	if(param.eType & MShader::MEShaderType::Pixel)
		m_pDevice->m_pD3dContext->PSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
}

void MDirectX11Renderer::SetVertexShaderTexture(MShaderTextureParam& param)
{
	if (param.pTexture)
	{
		if (nullptr == param.pTexture->GetBuffer())
		{
			param.pTexture->GenerateBuffer(m_pDevice);
		}

		m_pDevice->m_pD3dContext->VSSetShaderResources(param.unBindPoint, param.unBindCount, &(param.pTexture->GetBuffer()->m_pShaderResourceView));
	}
}

void MDirectX11Renderer::SetPixelShaderTexture(MShaderTextureParam& param)
{
	if (param.pTexture)
	{
		if (nullptr == param.pTexture->GetBuffer())
		{
			param.pTexture->GenerateBuffer(m_pDevice);
		}

		m_pDevice->m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &(param.pTexture->GetBuffer()->m_pShaderResourceView));
	}
	else
	{
		static ID3D11ShaderResourceView* pNullPtr = nullptr;
		m_pDevice->m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &pNullPtr);
	}
}


#endif