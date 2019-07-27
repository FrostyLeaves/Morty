#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"

MDirectX11Renderer::MDirectX11Renderer()
	: m_pD3dDevice(nullptr)
	, m_pD3dContext(nullptr)
{

}

MDirectX11Renderer::~MDirectX11Renderer()
{

}

void MDirectX11Renderer::AddOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{

		ID3D11Texture2D* pBackBuffer = nullptr;

		RenderTarget rt;

		rt.pRenderView = pWindowView;
		rt.pSwapChain = this->CreateSwapChainForWindow(pWindowView->GetHWND());
		rt.pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));

		m_pD3dDevice->CreateRenderTargetView(pBackBuffer, 0, &rt.pTargetView);
		pBackBuffer->Release();

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
				m_vRenderTargets.erase(iter);
				break;
			}
		}
	}
}

bool MDirectX11Renderer::InitDirectX11()
{
	UINT nCreateDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	nCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL nFeatureLevel;
	ID3D11Device* pD3dDevice = nullptr;
	ID3D11DeviceContext* pD3DContext = nullptr;

	// 依次找支持的硬件设备，WARP设备(高效的CPU渲染设备)、软件驱动设备、引用设备，找着哪个用哪个，它们四个的运行效率依次降低。
	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
	};

	unsigned int nTotalDriverTypes = ARRAYSIZE(driverTypes);

	//依次找支持D3D11、支持D3D10.1、支持D3D10.0的硬件设备，找到哪个用哪个，找不到就用WARP。
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	unsigned int nTotalFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr;
	for (unsigned int driverTypeIndex = 0; driverTypeIndex < nTotalDriverTypes; ++driverTypeIndex)
	{
		hr = D3D11CreateDevice(
			0,										//默认显示适配器
			driverTypes[driverTypeIndex],			//使用支持的设备
			0,
			nCreateDeviceFlags,
			featureLevels,
			nTotalFeatureLevels,
			D3D11_SDK_VERSION,
			&pD3dDevice,
			&nFeatureLevel,
			&pD3DContext
			);

		if (SUCCEEDED(hr))
		{
			m_pD3dDevice = pD3dDevice;
			m_pD3dContext = pD3DContext;
			m_nFeatureLevel = nFeatureLevel;
			m_nDriverType = driverTypes[driverTypeIndex];

			pD3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_n4xMsaaQuality);

			return true;
		}
	}

	if (FAILED(hr))
	{
		DXTRACE_MSG("Failed to create the Direct3D device!");
		return false;
	}

	return true;
}

bool MDirectX11Renderer::Initialize()
{
	if (nullptr == m_pD3dDevice && false == InitDirectX11())
		return false;

	return true;

}

void MDirectX11Renderer::Release()
{
	for (auto target : m_vRenderTargets)
	{
		target.pSwapChain->Release();
		target.pTargetView->Release();
	}


	if (m_pD3dContext)
		m_pD3dContext->Release();

	if (m_pD3dDevice)
		m_pD3dDevice->Release();

	m_pD3dContext = nullptr;
	m_pD3dDevice = nullptr;

}

void MDirectX11Renderer::RenderNodeToView(MNode* pNode, MIRenderView* pView)
{
	if (!m_pD3dContext)
		return;


	IDXGISwapChain* pSwapChain = nullptr;
	ID3D11RenderTargetView* pTargetView = nullptr;
	for (auto rt : m_vRenderTargets)
	{
		if (rt.pRenderView == pView)
		{
			pSwapChain = rt.pSwapChain;
			pTargetView = rt.pTargetView;
			break;
		}
	}

	if (pSwapChain == nullptr || pTargetView == nullptr)
		return;



	float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
	m_pD3dContext->ClearRenderTargetView(pTargetView, clearColor);
	pSwapChain->Present(0, 0);
}

IDXGISwapChain* MDirectX11Renderer::CreateSwapChainForWindow(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;

	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	bool bEnable4xMsaa = false;

	if (bEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_n4xMsaaQuality;
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
	m_pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);

	IDXGIAdapter* pDxgiAdapter = nullptr;
	pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);

	IDXGIFactory* pDxgiFactory = nullptr;
	pDxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDxgiFactory);

	IDXGISwapChain* pSwapChain = nullptr;
	pDxgiFactory->CreateSwapChain(m_pD3dDevice, &sd, &pSwapChain);

	pDxgiDevice->Release();
	pDxgiAdapter->Release();
	pDxgiFactory->Release();


	return pSwapChain;
}
