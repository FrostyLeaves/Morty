#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"
#include "MLogManager.h"

#include "D3Dcompiler.h"
#include "d3d11shader.h"

#include "MDirectX11Device.h"

#include "MTexture.h"
#include "MShader.h"
#include "MMaterial.h"
#include "MMesh.h"
#include "MIScene.h"

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

const bool bEnable4xMsaa = false;

MDirectX11Renderer::MDirectX11Renderer(MDirectX11Device* pDevice)
	: m_pDevice(pDevice)
	, m_pDefaultSamplerState(nullptr)
	, m_pAnisotropicFilterSamplerState(nullptr)
	, m_pDepthStencilState(nullptr)
{

}

MDirectX11Renderer::~MDirectX11Renderer()
{

}

void MDirectX11Renderer::AddOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{
		RenderTarget rt = this->CreateRenderTargetForWindow(pView);
		OnResize(rt, DEFAULT_WIDTH, DEFAULT_HEIGHT);
		m_vRenderTargets.push_back(rt);

	}
}

void MDirectX11Renderer::RemoveOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{
		for (std::vector<RenderTarget>::iterator iter = m_vRenderTargets.begin(); iter != m_vRenderTargets.end(); ++iter)
		{
			if ((*iter).pRenderView == pWindowView)
			{
				(*iter).pSwapChain->Release();
				(*iter).pTargetView->Release();
				if (iter->pDepthStencilBuffer)
					iter->pDepthStencilBuffer->Release();
				if (iter->pDepthStencilView)
					iter->pDepthStencilView->Release();
				m_vRenderTargets.erase(iter);
				break;
			}
		}
	}
}

bool MDirectX11Renderer::Initialize()
{

	//光栅化状态块
	D3D11_RASTERIZER_DESC mRasterizer;
	ZeroMemory(&mRasterizer, sizeof(D3D11_RASTERIZER_DESC));
	//实心模式，WIREFRAME是线框模式
	mRasterizer.FillMode = D3D11_FILL_SOLID;
	mRasterizer.CullMode = D3D11_CULL_NONE;
	mRasterizer.FrontCounterClockwise = false;
	mRasterizer.DepthClipEnable = true;//这个裁剪指的是平截头裁剪；

	//TODO 如果在一个程序中，需要多种光栅化状态块来回切换，那么在初始化时就创建好，而不是切换的时候创建。

	m_pRasterizerState = nullptr;
	HRESULT hr = m_pDevice->m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create RasterizerState! Initialize return false.");
		if (m_pRasterizerState)
		{
			m_pRasterizerState->Release();
			m_pRasterizerState = nullptr;
		}
		return false;
	}

	m_pDevice->m_pD3dContext->RSSetState(m_pRasterizerState);

	//三角形解析顶点
	m_pDevice->m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


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

	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = m_pDevice->m_pD3dDevice->CreateSamplerState(&sampDesc, &m_pAnisotropicFilterSamplerState);


	//深度状态
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	// 允许使用深度值一致的像素进行替换的深度/模板状态
	// 该状态用于绘制天空盒，因为深度值为1.0时默认无法通过深度测试
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	dsDesc.StencilEnable = false;

	m_pDevice->m_pD3dDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);

	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_pDepthStencilState, 0);



	InitDefaultResource();

	return true;
}

void MDirectX11Renderer::Release()
{
	for (auto target : m_vRenderTargets)
	{
		target.pSwapChain->Release();
		target.pTargetView->Release();
		target.pDepthStencilBuffer->Release();
		target.pDepthStencilView->Release();
	}

	if (m_pRasterizerState)
		m_pRasterizerState->Release();

	if (m_pDefaultSamplerState)
		m_pDefaultSamplerState->Release();
}

void MDirectX11Renderer::RenderSceneToView(MIScene* pScene, MIRenderView* pView)
{
	if (!m_pDevice->m_pD3dContext)
		return;

	RenderTarget target;
	for (auto rt : m_vRenderTargets)
	{
		if (rt.pRenderView == pView)
		{
			target = rt;
			break;
		}
	}

	if (target.pSwapChain == nullptr || target.pTargetView == nullptr)
		return;


	float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
	m_pDevice->m_pD3dContext->ClearDepthStencilView(target.pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDevice->m_pD3dContext->ClearRenderTargetView(target.pTargetView, clearColor);
	m_pDevice->m_pD3dContext->RSSetViewports(1, &target.mViewport);
	m_pDevice->m_pD3dContext->OMSetRenderTargets(1, &target.pTargetView, target.pDepthStencilView);
	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_pDepthStencilState, 0);

	pView->OnRenderBegin();

	
	if (MIScene* pScene = pView->GetScene())
	{
		pScene->Render(this, pView);
	}


	pView->OnRenderEnd();

	target.pSwapChain->Present(0, 0);



}

void MDirectX11Renderer::InitDefaultResource()
{
	m_pDefaultTexture = new MTexture();
	m_pDefaultTexture->SetSize(Vector2(1, 1));
	m_pDefaultTexture->GetImageData()[0] = 0;
	m_pDefaultTexture->GetImageData()[1] = 0;
	m_pDefaultTexture->GetImageData()[2] = 0;
	m_pDefaultTexture->GetImageData()[3] = 255;
	m_pDefaultTexture->GenerateBuffer(m_pDevice);

}

void MDirectX11Renderer::ReleaseDefaultResource()
{
	//TODO

	if (m_pDefaultTexture)
	{
		m_pDefaultTexture->DestroyTexture(m_pDevice);
		delete m_pDefaultTexture;
		m_pDefaultTexture = nullptr;
	}
}

MDirectX11Renderer::RenderTarget MDirectX11Renderer::CreateRenderTargetForWindow(MIRenderView* pView)
{
	RenderTarget result;
	MWindowsRenderView* pRenderView = dynamic_cast<MWindowsRenderView*>(pView);
	if (nullptr == pRenderView)
	{
		return result;
	}

	HWND hWnd = pRenderView->GetHWND();



	DXGI_SWAP_CHAIN_DESC sd;

	sd.BufferDesc.Width = DEFAULT_WIDTH;
	sd.BufferDesc.Height = DEFAULT_HEIGHT;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (bEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_pDevice->m_n4xMsaaQuality;
	}
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	IDXGIDevice* pDxgiDevice = nullptr;
	m_pDevice->m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);

	IDXGIAdapter* pDxgiAdapter = nullptr;
	pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);

	IDXGIFactory* pDxgiFactory = nullptr;
	pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDxgiFactory);

	IDXGISwapChain* pSwapChain = nullptr;
	pDxgiFactory->CreateSwapChain(m_pDevice->m_pD3dDevice, &sd, &pSwapChain);

	pDxgiDevice->Release();
	pDxgiAdapter->Release();
	pDxgiFactory->Release();

	result.pRenderView = pRenderView;
	result.pSwapChain = pSwapChain;


	D3D11_VIEWPORT& viewport = result.mViewport;
	viewport.Width = DEFAULT_WIDTH;
	viewport.Height = DEFAULT_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	return result;
}

void MDirectX11Renderer::OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight)
{
	for (auto& rt : m_vRenderTargets)
	{
		if (rt.pRenderView == pView)
		{
			OnResize(rt, nWidth, nHeight);
			break;
		}
	}


}

void MDirectX11Renderer::OnResize(RenderTarget& rt, const int& nWidth, const int& nHeight)
{
	if (rt.pTargetView)
	{
		rt.pTargetView->Release();
		rt.pTargetView = nullptr;
	}

	if (rt.pDepthStencilBuffer)
	{
		rt.pDepthStencilBuffer->Release();
		rt.pDepthStencilBuffer = nullptr;
	}

	if (rt.pDepthStencilView)
	{
		rt.pDepthStencilView->Release();
		rt.pDepthStencilView = nullptr;
	}

	// Resize the swap chain and recreate the render target view.
	HRESULT hr;


	hr = rt.pSwapChain->ResizeBuffers(1, nWidth, nHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to ResizeBuffers!");
		return;
	}

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = rt.pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to GetBuffer!");
		return;
	}

	hr = m_pDevice->m_pD3dDevice->CreateRenderTargetView(pBackBuffer, 0, &rt.pTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateRenderTargetView!");
		return;
	}

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = nWidth;
	depthStencilDesc.Height = nHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (bEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_pDevice->m_n4xMsaaQuality;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	hr = m_pDevice->m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &rt.pDepthStencilBuffer);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateTexture2D!");
		return;
	}

	//深度缓存视图
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = depthStencilDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = m_pDevice->m_pD3dDevice->CreateDepthStencilView(rt.pDepthStencilBuffer, 0, &rt.pDepthStencilView);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateDepthStencilView!");
		return;
	}

	// Bind the render target view and depth/stencil view to the pipeline.
	m_pDevice->m_pD3dContext->OMSetRenderTargets(1, &rt.pTargetView, rt.pDepthStencilView);

	rt.mViewport.Width = (float)nWidth;
	rt.mViewport.Height = (float)nHeight;

}

void MDirectX11Renderer::SetUseMaterial(MMaterial* pMaterial)
{

	if (nullptr == pMaterial)
	{
		//TODO 使用默认材质
		return;
	}

	MShader* pVertexShader = pMaterial->GetVertexShader();
	MShader* pPixelShader = pMaterial->GetPixelShader();

	if (nullptr == pVertexShader || nullptr == pPixelShader)
	{
		//TODO 使用默认材质
		return;
	}

	if (nullptr == pVertexShader->GetBuffer())
	{
		pVertexShader->CompileShader(m_pDevice);
		pMaterial->CompileVertexShaderParams();
	}
	if (nullptr == pPixelShader->GetBuffer())
	{
		pPixelShader->CompileShader(m_pDevice);
		pMaterial->CompilePixelShaderParams();
	}

	if (MVertexShaderBuffer* pVertexShaderBuffer = dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer()))
	{
		if (pVertexShaderBuffer->m_pInputLayout)
		{
			m_pDevice->m_pD3dContext->IASetInputLayout(pVertexShaderBuffer->m_pInputLayout);
		}
	}

	m_pDevice->m_pD3dContext->VSSetShader(dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer())->m_pVertexShader, nullptr, 0);
	m_pDevice->m_pD3dContext->PSSetShader(dynamic_cast<MPixelShaderBuffer*>(pPixelShader->GetBuffer())->m_pPixelShader, nullptr, 0);


	for (MShaderParam& param : pMaterial->GetVertexShaderParams())
	{
		UpdateShaderParam(param);
		m_pDevice->m_pD3dContext->VSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

	for (MShaderParam& param : pMaterial->GetPixelShaderParams())
	{
		UpdateShaderParam(param);
		m_pDevice->m_pD3dContext->PSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

	for (MShaderTextureParam& param : pMaterial->GetPixelTextureParams())
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

	for (MShaderSampleParam& param : pMaterial->GetPixelShader()->GetBuffer()->m_vSampleParamsTemplate)
	{
		m_pDevice->m_pD3dContext->PSSetSamplers(param.unBindPoint, param.unBindCount, &m_pAnisotropicFilterSamplerState);
	}
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
}

