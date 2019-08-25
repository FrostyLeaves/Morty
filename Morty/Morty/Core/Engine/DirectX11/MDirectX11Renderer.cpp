#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"

#include "MVertex.h"
#include "MMesh.h"

const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 600;

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

	//光栅化状态块
	D3D11_RASTERIZER_DESC mRasterizer;
	ZeroMemory(&mRasterizer, sizeof(D3D11_RASTERIZER_DESC));
	//实心模式，WIREFRAME是线框模式
	mRasterizer.FillMode = D3D11_FILL_SOLID;
	mRasterizer.CullMode = D3D11_CULL_BACK;
	mRasterizer.FrontCounterClockwise = false;
	mRasterizer.DepthClipEnable = true;

	//TODO 如果在一个程序中，需要多种光栅化状态块来回切换，那么在初始化时就创建好，而不是切换的时候创建。

	m_pRasterizerState = nullptr;
	HRESULT hr = m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState);

	m_pD3dContext->RSSetState(m_pRasterizerState);


	return true;

}

void MDirectX11Renderer::Release()
{
	for (auto target : m_vRenderTargets)
	{
		target.pSwapChain->Release();
		target.pTargetView->Release();
	}

	if (m_pRasterizerState)
		m_pRasterizerState->Release();

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

	result.pRenderView = pRenderView;
	result.pSwapChain = pSwapChain;

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
		//TODO
	}

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = rt.pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{

	}

	hr = m_pD3dDevice->CreateRenderTargetView(pBackBuffer, 0, &rt.pTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{

	}

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = nWidth;
	depthStencilDesc.Height = nHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	bool bEnable4xMsaa = false;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (bEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_n4xMsaaQuality;
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


	hr = m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &rt.pDepthStencilBuffer);
	if (FAILED(hr))
	{

	}

	hr = m_pD3dDevice->CreateDepthStencilView(rt.pDepthStencilBuffer, 0, &rt.pDepthStencilView);
	if (FAILED(hr))
	{

	}

	// Bind the render target view and depth/stencil view to the pipeline.
	m_pD3dContext->OMSetRenderTargets(1, &rt.pTargetView, rt.pDepthStencilView);

} 

void MDirectX11Renderer::GenerateBuffer(MVertexBuffer** pVertexBuffer, MMesh* pMesh)
{

	D3D11_INPUT_ELEMENT_DESC desc[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	
	//TODO CreateInputLayout need a virtual shader.
	ID3D11InputLayout* pInputLayout = nullptr;
	HRESULT hr = m_pD3dDevice->CreateInputLayout(desc, sizeof(desc), nullptr, 0, &pInputLayout);


	//m_pD3dContext->IASetInputLayout(pInputLayout);




	//创建顶点缓冲
	D3D11_BUFFER_DESC bufferDesc;

	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = pMesh->GetVerticesLength() * sizeof(MVertex);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = pMesh->GetVertices();

	ID3D11Buffer* pVB = nullptr;
	hr = m_pD3dDevice->CreateBuffer(&bufferDesc, &subResourceData, &pVB);

	//UINT stride = sizeof(MVertex);
	//UINT offset = 0;
	//m_pD3dContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
	

	//创建索引缓冲
	D3D11_BUFFER_DESC indicesBufferDesc;
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.ByteWidth = sizeof(unsigned int) * pMesh->GetIndicesLength();
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.CPUAccessFlags = 0;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indicesData;
	indicesData.pSysMem = pMesh->GetIndices();

	ID3D11Buffer* pIndicesBuffer = nullptr;
	hr = m_pD3dDevice->CreateBuffer(&indicesBufferDesc, &indicesData, &pIndicesBuffer);

	//m_pD3dContext->IASetIndexBuffer(pIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);

	//
	m_pD3dContext->DrawIndexed()
}

void MDirectX11Renderer::DestroyBuffer(MVertexBuffer** pVertexBuffer)
{

}

void MDirectX11Renderer::Draw(MVertexBuffer* pBuffer)
{
	m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



}
