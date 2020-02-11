#include "MDirectX11Renderer.h"
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

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

const bool bEnable4xMsaa = true;

MDirectX11Renderer::MDirectX11Renderer(MDirectX11Device* pDevice)
	: m_pDevice(pDevice)
	, m_pDefaultSamplerState(nullptr)
	, m_pDepthTextureSamplerState(nullptr)
//	, m_pAnisotropicFilterSamplerState(nullptr)
	, m_pDepthStencilState(nullptr)
	, m_pRasterizerState_Wireframe_CullNone(nullptr)
	, m_pRasterizerState_Solid_CullNone(nullptr)
	, m_pRasterizerState_Solid_CullBack(nullptr)
	, m_pRasterizerState_Solid_CullFront(nullptr)
	, m_pUsingMaterial(nullptr)
{

}

MDirectX11Renderer::~MDirectX11Renderer()
{

}

void MDirectX11Renderer::AddOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{
		MDirectX11RenderTarget* pRenderTarget = MDirectX11RenderTarget::CreateForView(m_pDevice, pWindowView);
		pView->SetRenderTarget(pRenderTarget);

	}
}

void MDirectX11Renderer::RemoveOutputView(MIRenderView* pView)
{
	MIRenderTarget* pRenderTarget = pView->GetRenderTarget();

	pView->SetRenderTarget(nullptr);
	delete pRenderTarget;
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

	HRESULT hr = m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState_Solid_CullBack);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create RasterizerState! Initialize return false.");
		if (m_pRasterizerState_Solid_CullNone)
		{
			m_pRasterizerState_Solid_CullNone->Release();
			m_pRasterizerState_Solid_CullNone = nullptr;
		}
		return false;
	}

	mRasterizer.CullMode = D3D11_CULL_FRONT;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState_Solid_CullFront);
	mRasterizer.CullMode = D3D11_CULL_NONE;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState_Solid_CullNone);
	mRasterizer.FillMode = D3D11_FILL_WIREFRAME;
	m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState_Wireframe_CullNone);


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


	//深度状态
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;

	m_pDevice->m_pD3dDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);
	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_pDepthStencilState, 0);


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
	m_pDevice->m_pD3dDevice->CreateSamplerState(&comparisonSamplerDesc, &m_pDepthTextureSamplerState);


	//三角形解析顶点
	m_pDevice->m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	InitDefaultResource();

	return true;
}

void MDirectX11Renderer::Release()
{
	if (m_pRasterizerState_Solid_CullFront)
	{
		m_pRasterizerState_Solid_CullFront->Release();
		m_pRasterizerState_Solid_CullFront = nullptr;
	}
	if (m_pRasterizerState_Solid_CullBack)
	{
		m_pRasterizerState_Solid_CullBack->Release();
		m_pRasterizerState_Solid_CullBack = nullptr;
	}
	if (m_pRasterizerState_Solid_CullNone)
	{
		m_pRasterizerState_Solid_CullNone->Release();
		m_pRasterizerState_Solid_CullNone = nullptr;
	}
	if (m_pRasterizerState_Wireframe_CullNone)
	{
		m_pRasterizerState_Wireframe_CullNone->Release();
		m_pRasterizerState_Wireframe_CullNone = nullptr;
	}
	if (m_pDepthStencilState)
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = nullptr;
	}
	if (m_pDefaultSamplerState)
	{
		m_pDefaultSamplerState->Release();
		m_pDefaultSamplerState = nullptr;
	}
	if (m_pDepthTextureSamplerState)
	{
		m_pDepthTextureSamplerState->Release();
		m_pDepthTextureSamplerState = nullptr;
	}

	ReleaseDefaultResource();
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

void MDirectX11Renderer::Render(MIRenderTarget* pRenderTarget)
{
	if (pRenderTarget)
	{
		if (!m_pDevice || !m_pDevice->m_pD3dContext)
			return;

		if(pRenderTarget->m_pDepthStencilView)
			m_pDevice->m_pD3dContext->ClearDepthStencilView(pRenderTarget->m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		if(pRenderTarget->m_pTargetView)
			m_pDevice->m_pD3dContext->ClearRenderTargetView(pRenderTarget->m_pTargetView, pRenderTarget->m_backgroundColor.m);


		m_vRenderTargets.push(pRenderTarget);
		RecoverRenderTarget(pRenderTarget);
		pRenderTarget->OnRender(this);
		m_vRenderTargets.pop();


		//恢复上一个渲染目标的状态
		if (!m_vRenderTargets.empty())
			RecoverRenderTarget(m_vRenderTargets.top());
	}

}

void MDirectX11Renderer::RecoverRenderTarget(MIRenderTarget* pRenderTarget)
{
	//warning! Material may has been switched.

	if(pRenderTarget->m_pTargetView)
		m_pDevice->m_pD3dContext->OMSetRenderTargets(1, &pRenderTarget->m_pTargetView, pRenderTarget->m_pDepthStencilView);
	else
		m_pDevice->m_pD3dContext->OMSetRenderTargets(0, nullptr, pRenderTarget->m_pDepthStencilView);
}

void MDirectX11Renderer::InitDefaultResource()
{
	m_pDefaultTexture = new MTexture();
	m_pDefaultTexture->SetSize(Vector2(2, 2));
	m_pDefaultTexture->GetImageData()[0] = 255;
	m_pDefaultTexture->GetImageData()[1] = 255;
	m_pDefaultTexture->GetImageData()[2] = 255;
	m_pDefaultTexture->GetImageData()[3] = 255;
	m_pDefaultTexture->GenerateBuffer(m_pDevice);
}

void MDirectX11Renderer::ReleaseDefaultResource()
{
	if (m_pDefaultTexture)
	{
		m_pDefaultTexture->DestroyTexture(m_pDevice);
		delete m_pDefaultTexture;
		m_pDefaultTexture = nullptr;
	}
}

bool MDirectX11Renderer::SetUseMaterial(MMaterial* pMaterial)
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


	if (m_eRasterizerType != pMaterial->GetRenderState())
	{
		//切换渲染状态
		m_eRasterizerType = pMaterial->GetRenderState();

		if (m_eRasterizerType & MERasterizerType::EWireframe)
			m_pDevice->m_pD3dContext->RSSetState(m_pRasterizerState_Wireframe_CullNone);
		else if (m_eRasterizerType & MERasterizerType::ECullBack)
			m_pDevice->m_pD3dContext->RSSetState(m_pRasterizerState_Solid_CullBack);
		else if (m_eRasterizerType & MERasterizerType::ECullFront)
			m_pDevice->m_pD3dContext->RSSetState(m_pRasterizerState_Solid_CullFront);
		else 
			m_pDevice->m_pD3dContext->RSSetState(m_pRasterizerState_Solid_CullNone);
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
	}
}

void MDirectX11Renderer::UpdateMaterialParam()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderParam& param : m_pUsingMaterial->GetVertexShaderParams())
	{
		if(param.bDirty)
			UpdateShaderParam(param);
		m_pDevice->m_pD3dContext->VSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

	for (MShaderParam& param : m_pUsingMaterial->GetPixelShaderParams())
	{
		if(param.bDirty)
			UpdateShaderParam(param);
		m_pDevice->m_pD3dContext->PSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

}

void MDirectX11Renderer::UpdateMaterialResource()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderTextureParam& param : m_pUsingMaterial->GetPixelTextureParams())
	{
		if (param.pTexture)
		{
			if (nullptr == param.pTexture->GetBuffer())
				param.pTexture->GenerateBuffer(m_pDevice);

			m_pDevice->m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &(param.pTexture->GetBuffer()->m_pShaderResourceView));
		}
		else
		{
			m_pDevice->m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &(m_pDefaultTexture->GetBuffer()->m_pShaderResourceView));
		}
	}

	for (MShaderSampleParam& param : m_pUsingMaterial->GetPixelShader()->GetBuffer()->m_vSampleParamsTemplate)
	{
		if (param.strName == "U_defaultSampler")
			m_pDevice->m_pD3dContext->PSSetSamplers(param.unBindPoint, param.unBindCount, &m_pDefaultSamplerState);
		else if (param.strName == "U_shadowMapSampler")
			m_pDevice->m_pD3dContext->PSSetSamplers(param.unBindPoint, param.unBindCount, &m_pDepthTextureSamplerState);
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

