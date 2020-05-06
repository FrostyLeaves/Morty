#include "MDirectX11RenderTarget.h"
#include "MWindowsRenderView.h"
#include "MDirectX11Device.h"
#include "MLogManager.h"
#include "MViewport.h"
#include "MTexture.h"

MDirectX11RenderTarget::MDirectX11RenderTarget(MDirectX11Device* pDevice)
	: MIRenderTarget()
	, m_pSwapChain(nullptr)
	, m_pDevice(pDevice)
	, m_pView(nullptr)
	, m_pDepthTexture(new MRenderDepthTexture())
{
}

MDirectX11RenderTarget::~MDirectX11RenderTarget()
{
	Release(m_pDevice);
}

MDirectX11RenderTarget* MDirectX11RenderTarget::CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView)
{
	if (nullptr == pDevice)
		return nullptr;

	HWND hWnd = pView->GetHWND();
	MDirectX11RenderTarget* pRenderTarget = new MDirectX11RenderTarget(pDevice);

	DXGI_SWAP_CHAIN_DESC sd;

	sd.BufferDesc.Width = pView->GetViewWidth() > 0 ? pView->GetViewWidth() : 1;
	sd.BufferDesc.Height = pView->GetViewHeight() > 0 ? pView->GetViewHeight() : 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	if (pDevice->m_bEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = pDevice->m_n4xMsaaQuality;
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
	pDevice->m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);

	IDXGIAdapter* pDxgiAdapter = nullptr;
	pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);

	IDXGIFactory* pDxgiFactory = nullptr;
	pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDxgiFactory);

	IDXGISwapChain* pSwapChain = nullptr;
	pDxgiFactory->CreateSwapChain(pDevice->m_pD3dDevice, &sd, &pSwapChain);

	pDxgiDevice->Release();
	pDxgiAdapter->Release();
	pDxgiFactory->Release();

	pRenderTarget->m_pSwapChain = pSwapChain;

	unsigned int unWidth = pView->GetViewWidth();
	unsigned int unHeight = pView->GetViewHeight();
	pRenderTarget->OnResize(unWidth, unHeight);


	pRenderTarget->m_pView = pView;
	pView->SetRenderTarget(pRenderTarget);

	return pRenderTarget;
}

void MDirectX11RenderTarget::OnResize(const unsigned int& unWidth, const unsigned int& unHeight)
{
	if (nullptr == m_pDevice)
		return;

	unsigned int unSafeWidth = unWidth;
	unsigned int unSafeHeight = unHeight;

	if (unSafeWidth < 1)
		unSafeWidth = 1;
	if (unSafeHeight < 1)
		unSafeHeight = 1;

	m_pDevice->DestroyRenderTarget(this);

	HRESULT hr;
// 
// 	DXGI_SWAP_CHAIN_DESC desc;
// 	m_pSwapChain->GetDesc(&desc);
// 	DXGI_MODE_DESC mode_desc;
// 	ZeroMemory(&mode_desc, sizeof(mode_desc));
// 
// 	mode_desc.Width = desc.BufferDesc.Width;
// 	mode_desc.Height = desc.BufferDesc.Height;
// 	mode_desc.RefreshRate.Numerator = desc.BufferDesc.RefreshRate.Numerator;
// 	mode_desc.RefreshRate.Denominator = desc.BufferDesc.RefreshRate.Denominator;
// 	mode_desc.Format = desc.BufferDesc.Format;
// 	mode_desc.Scaling = desc.BufferDesc.Scaling;
// 
// 	hr = m_pSwapChain->ResizeTarget(&mode_desc);
// 	if (FAILED(hr))
// 	{
// 		MLogManager::GetInstance()->Error("Failed to ResizeTarget!");
// 		return;
// 	}
// 	m_pSwapChain->SetFullscreenState(true, nullptr);

	hr = m_pSwapChain->ResizeBuffers(1, unSafeWidth, unSafeHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to ResizeBuffers!");
		return;
	}

	m_pDevice->GenerateRenderTarget(this, unSafeWidth, unSafeHeight);


//	MIRenderTarget::OnResize(unWidth, unHeight);
}

void MDirectX11RenderTarget::OnRender(MIRenderer* pRenderer)
{
	if (nullptr == m_pView)
		return;

	m_pView->OnRenderBegin();
	for (MViewport* pViewport : m_pView->GetViewports())
	{
		pViewport->Render(pRenderer);
	}
	m_pView->OnRenderEnd();

	m_pSwapChain->Present(0, 0);
}

void MDirectX11RenderTarget::Release(MIDevice* pDevice)
{
	m_pDevice->DestroyRenderTarget(this);
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pDepthTexture)
	{
		m_pDepthTexture->DestroyTexture(pDevice);
		delete m_pDepthTexture;
		m_pDepthTexture = nullptr;
	}

//	MTextureRenderTarget::Release(pDevice);
}
