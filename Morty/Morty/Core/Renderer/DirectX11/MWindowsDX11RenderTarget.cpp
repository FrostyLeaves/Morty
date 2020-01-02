#include "MWindowsDX11RenderTarget.h"
#include "MWindowsRenderView.h"
#include "MDirectX11Device.h"
#include "MLogManager.h"

MWindowsDX11RenderTarget::MWindowsDX11RenderTarget(MDirectX11Device* pDevice) : MIRenderTarget()
	,m_pDevice(pDevice), m_pSwapChain(nullptr), m_funcRenderFunction(nullptr)
{

}

MWindowsDX11RenderTarget::~MWindowsDX11RenderTarget()
{
	if (m_pDevice)
		m_pDevice->DestroyRenderTarget(this);

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
}

MWindowsDX11RenderTarget* MWindowsDX11RenderTarget::CreateForView(MDirectX11Device* pDevice, MWindowsRenderView* pView)
{
	if (nullptr == pDevice)
		return nullptr;

	HWND hWnd = pView->GetHWND();
	MWindowsDX11RenderTarget* pRenderTarget = new MWindowsDX11RenderTarget(pDevice);


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

	pRenderTarget->OnResize(pView->GetViewWidth(), pView->GetViewHeight());
	return pRenderTarget;
}

void MWindowsDX11RenderTarget::OnResize(int nWidth, int nHeight)
{
	if (nullptr == m_pDevice)
		return;

	if (nWidth < 1)
		nWidth = 1;
	if (nHeight < 1)
		nHeight = 1;

	m_pDevice->DestroyRenderTarget(this);

	HRESULT hr;
	hr = m_pSwapChain->ResizeBuffers(1, nWidth, nHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to ResizeBuffers!");
		return;
	}

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to GetBuffer!");
		return;
	}

	
	this->m_pBackBuffer = pBackBuffer;

	m_pDevice->GenerateRenderTarget(this, nWidth, nHeight);
}

void MWindowsDX11RenderTarget::OnRender(MIRenderer* pRenderer)
{
	MIRenderTarget::OnRender(pRenderer);
	m_pSwapChain->Present(0, 0);
}
