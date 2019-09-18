#include "MDirectX11Renderer.h"
#include "MWindowsRenderView.h"
#include "MLogManager.h"

#include "D3Dcompiler.h"
#include "d3d11shader.h"

#include "MVertex.h"
#include "MMesh.h"
#include "MShader.h"
#include "MMaterial.h"
#include "MMeshInstance.h"
#include "MModelResource.h"
#include "MModel.h"
#include "MCamera.h"
#include "MTexture.h"

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

const bool bEnable4xMsaa = false;

MDirectX11Renderer::MDirectX11Renderer()
	: m_pD3dDevice(nullptr)
	, m_pD3dContext(nullptr)
	, m_pDefaultSamplerState(nullptr)
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
		MLogManager::GetInstance()->Error("Failed to create the Direct3D device!");
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
	mRasterizer.CullMode = D3D11_CULL_NONE;
	mRasterizer.FrontCounterClockwise = false;
	mRasterizer.DepthClipEnable = true;//这个裁剪指的是平截头裁剪；

	//TODO 如果在一个程序中，需要多种光栅化状态块来回切换，那么在初始化时就创建好，而不是切换的时候创建。

	m_pRasterizerState = nullptr;
	HRESULT hr = m_pD3dDevice->CreateRasterizerState(&mRasterizer, &m_pRasterizerState);
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

	m_pD3dContext->RSSetState(m_pRasterizerState);

	//三角形解析顶点
	m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


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
	hr = m_pD3dDevice->CreateSamplerState(&sampDesc, &m_pDefaultSamplerState);


	InitDefaultResource();

	return true;
}

ID3D11InputLayout* MDirectX11Renderer::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const int& nLength)
{
	ID3D10Blob* pErrorMessage = nullptr;
	ID3D10Blob* pShaderBuffer = nullptr;

	const MString strVirtualShaderFront = "struct VertexIn {	\n\ ";
	const MString strVirtualShaderBack = "};	\n struct VertexOut	\n {	\n float4 PosH : SV_POSITION;\n float Color : COLOR;	\n };	\n VertexOut VS(VertexIn vin)	\n {	\n VertexOut vout;	\n vout.PosH = float4(1.0f, 1.0f, 1.0f, 1.0f);\n return vout;	\n }";
	
	MString strVirtualShader = strVirtualShaderFront;
	for (int i = 0; i < nLength; ++i)
	{
		MString strType;
		switch (desc[i].Format)
		{
		case DXGI_FORMAT_R32G32B32_FLOAT:
			strType = "float3";
			break;
		case DXGI_FORMAT_R32G32_FLOAT:
			strType = "float2";
			break;
		default:
			strType = "float";
			break;
		}

		strVirtualShader += strType + "    v_" + std::to_string(i) + " : " + desc[i].SemanticName + " ; \n";
	}
	
	strVirtualShader += strVirtualShaderBack;
	
	HRESULT hr = D3DX11CompileFromMemory(strVirtualShader.c_str(), strVirtualShader.size(), nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, nullptr, &pShaderBuffer, &pErrorMessage, nullptr);
	if (FAILED(hr))
	{
		if (pErrorMessage)
			MLogManager::GetInstance()->Error("Compile Shader Error: %s", pErrorMessage->GetBufferPointer());
		else
			MLogManager::GetInstance()->Error("Compile Shader Error: Can`t find file: virtual shader");

		if (pShaderBuffer)
			pShaderBuffer->Release();

		return nullptr;
	}

	ID3D11InputLayout* pVertexInputLayout = nullptr;
	//TODO 需要根据顶点数据自动创建出假的Shader，来骗过DX11的Shader-InputLayout验证，让它认为Layout合法。
	hr = m_pD3dDevice->CreateInputLayout(desc, nLength, pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), &pVertexInputLayout);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create RasterizerState! Initialize return false.");
		if (pVertexInputLayout)
		{
			pVertexInputLayout->Release();
			pVertexInputLayout = nullptr;
		}

		return nullptr;
	}

	pShaderBuffer->Release();

	return pVertexInputLayout;
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

void MDirectX11Renderer::RenderNodeToView(MNode* pRootNode, MCamera* pCamera, MIRenderView* pView)
{
	if (!m_pD3dContext)
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
	m_pD3dContext->ClearDepthStencilView(target.pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pD3dContext->ClearRenderTargetView(target.pTargetView, clearColor);
	m_pD3dContext->RSSetViewports(1, &target.mViewport);
	m_pD3dContext->OMSetRenderTargets(1, &target.pTargetView, target.pDepthStencilView);
	m_pD3dContext->OMSetDepthStencilState(nullptr, 0);

	pView->OnRenderBegin();

	Matrix4 camTransInv = IdentityMatrix;
	if(pCamera)
		camTransInv = pCamera->GetWorldTransform().Inverse();
	Matrix4 projMat = Matrix4::MatrixPerspectiveFovLH(45, (float)pView->GetViewWidth() / pView->GetViewHeight(), 0.10f, 500);
	DrawNode(pRootNode, camTransInv * projMat);

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
	m_pDefaultTexture->GenerateBuffer(this);
}

void MDirectX11Renderer::ReleaseDefaultResource()
{
	//TODO
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

	hr = m_pD3dDevice->CreateRenderTargetView(pBackBuffer, 0, &rt.pTargetView);
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
		MLogManager::GetInstance()->Error("Failed to CreateTexture2D!");
		return;
	}

	//深度缓存视图
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = depthStencilDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = m_pD3dDevice->CreateDepthStencilView(rt.pDepthStencilBuffer, 0, &rt.pDepthStencilView);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateDepthStencilView!");
		return;
	}

	// Bind the render target view and depth/stencil view to the pipeline.
	m_pD3dContext->OMSetRenderTargets(1, &rt.pTargetView, rt.pDepthStencilView);

	rt.mViewport.Width = (float)nWidth;
	rt.mViewport.Height = (float)nHeight;

}

void MDirectX11Renderer::GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable/* = false*/)
{
	//创建顶点缓冲
	D3D11_BUFFER_DESC bufferDesc;

	if (bModifiable)
	{
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.CPUAccessFlags = 0;
	}
	bufferDesc.ByteWidth = pMesh->GetVerticesLength() * pMesh->GetVertexStructSize();
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = pMesh->GetVertices();

	ID3D11Buffer* pVB = nullptr;
	HRESULT hr = m_pD3dDevice->CreateBuffer(&bufferDesc, &subResourceData, &pVB);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateBuffer!");
		return;
	}

	//UINT stride = sizeof(MVertex);
	//UINT offset = 0;
	//m_pD3dContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);


	//创建索引缓冲
	D3D11_BUFFER_DESC indicesBufferDesc;
	if (bModifiable)
	{
		indicesBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indicesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indicesBufferDesc.CPUAccessFlags = 0;
	}
	indicesBufferDesc.ByteWidth = sizeof(unsigned int) * pMesh->GetIndicesLength();
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indicesData;
	indicesData.pSysMem = pMesh->GetIndices();

	ID3D11Buffer* pIB = nullptr;
	hr = m_pD3dDevice->CreateBuffer(&indicesBufferDesc, &indicesData, &pIB);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateBuffer!");
		return;
	}

	//m_pD3dContext->IASetIndexBuffer(pIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);

	//m_pD3dContext->DrawIndexed()

	if (*ppVertexBuffer)
	{
		DestroyBuffer(ppVertexBuffer);
	}
	(*ppVertexBuffer) = new MVertexBuffer();
	(*ppVertexBuffer)->m_pVertexBuffer = pVB;
	(*ppVertexBuffer)->m_pIndexBuffer = pIB;


}

void MDirectX11Renderer::GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap)
{
	Vector2 size = pTexture->GetSize();
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = (unsigned int)size.x;
	desc.Height = (unsigned int)size.y;
	desc.MipLevels = bGenerateMipmap ? 0 : 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;


	ID3D11Texture2D* pTextureBuffer = nullptr;

	if (bGenerateMipmap)
	{
		m_pD3dDevice->CreateTexture2D(&desc, nullptr, &pTextureBuffer);
		pTextureBuffer->GetDesc(&desc);
		m_pD3dContext->UpdateSubresource(pTextureBuffer, 0, nullptr, pTexture->GetImageData(), (unsigned int)size.x * 4, 0);

	}
	else
	{
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = pTexture->GetImageData();
		data.SysMemPitch = (unsigned int)size.x * 4;
		data.SysMemSlicePitch = (unsigned int)size.x * size.y * 4;
		m_pD3dDevice->CreateTexture2D(&desc, &data, &pTextureBuffer);
	}

	

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = bGenerateMipmap ? -1 : 1;

	ID3D11ShaderResourceView* pShaderResourceView = nullptr;
	HRESULT hr = m_pD3dDevice->CreateShaderResourceView(pTextureBuffer, &viewDesc, &pShaderResourceView);

	if (*ppTextureBuffer)
		DestroyTexture(ppTextureBuffer);
	
	(*ppTextureBuffer) = new MTextureBuffer();

	if (FAILED(hr))
	{
		if (pTextureBuffer)
			pTextureBuffer->Release();
		if (pShaderResourceView)
			pShaderResourceView->Release();
		
		MLogManager::GetInstance()->Error("Create Texture Buffer Error.");
	}
	else
	{
		if (bGenerateMipmap)
		{
			m_pD3dContext->GenerateMips(pShaderResourceView);
		}
		pTextureBuffer->Release();

		(*ppTextureBuffer)->m_pShaderResourceView = pShaderResourceView;
	}
}

void MDirectX11Renderer::DestroyBuffer(MVertexBuffer** ppVertexBuffer)
{
	if ((*ppVertexBuffer)->m_pVertexBuffer)
	{
		(*ppVertexBuffer)->m_pVertexBuffer->Release();
		(*ppVertexBuffer)->m_pVertexBuffer = nullptr;
	}
	if ((*ppVertexBuffer)->m_pIndexBuffer)
	{
		(*ppVertexBuffer)->m_pIndexBuffer->Release();
		(*ppVertexBuffer)->m_pIndexBuffer = nullptr;
	}
	delete *ppVertexBuffer;
	*ppVertexBuffer = nullptr;
}

void MDirectX11Renderer::DestroyTexture(MTextureBuffer** ppTextureBuffer)
{
	if ((*ppTextureBuffer)->m_pShaderResourceView)
	{
		(*ppTextureBuffer)->m_pShaderResourceView->Release();
		(*ppTextureBuffer)->m_pShaderResourceView = nullptr;
	}

	delete *ppTextureBuffer;
	*ppTextureBuffer = nullptr;
}

void MDirectX11Renderer::UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh)
{
	// Upload vertex/index data into a single contiguous GPU buffer
	D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
	if (m_pD3dContext->Map((*ppVertexBuffer)->m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
		return;
	if (m_pD3dContext->Map((*ppVertexBuffer)->m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
		return;

	memcpy(vtx_resource.pData, pMesh->GetVertices(), pMesh->GetVerticesLength() * sizeof(MVertex));
	memcpy(idx_resource.pData, pMesh->GetIndices(), sizeof(unsigned int) * pMesh->GetIndicesLength());

	m_pD3dContext->Unmap((*ppVertexBuffer)->m_pVertexBuffer, 0);
	m_pD3dContext->Unmap((*ppVertexBuffer)->m_pIndexBuffer, 0);
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
		pVertexShader->CompileShader(this);
		pMaterial->CompileVertexShaderParams();
	}
	if (nullptr == pPixelShader->GetBuffer())
	{
		pPixelShader->CompileShader(this);
		pMaterial->CompilePixelShaderParams();
	}

	if (MVertexShaderBuffer* pVertexShaderBuffer = dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer()))
	{
		if (pVertexShaderBuffer->m_pInputLayout)
		{
			m_pD3dContext->IASetInputLayout(pVertexShaderBuffer->m_pInputLayout);
		}
	}

	m_pD3dContext->VSSetShader(dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer())->m_pVertexShader, nullptr, 0);
	m_pD3dContext->PSSetShader(dynamic_cast<MPixelShaderBuffer*>(pPixelShader->GetBuffer())->m_pPixelShader, nullptr, 0);


	for (MShaderParam& param : pMaterial->GetVertexShaderParams())
	{
		UpdateShaderParam(param);
		m_pD3dContext->VSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

	for (MShaderParam& param : pMaterial->GetPixelShaderParams())
	{
		UpdateShaderParam(param);
		m_pD3dContext->PSSetConstantBuffers(param.unBindPoint, param.unBindCount, &param.pBuffer);
	}

	for (MShaderTextureParam& param : pMaterial->GetPixelTextureParams())
	{
		if (param.pTexture)
		{
			if (nullptr == param.pTexture->GetBuffer())
				param.pTexture->GenerateBuffer(this);

			m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &(param.pTexture->GetBuffer()->m_pShaderResourceView));
		}
		else
		{
			m_pD3dContext->PSSetShaderResources(param.unBindPoint, param.unBindCount, &(m_pDefaultTexture->GetBuffer()->m_pShaderResourceView));
		}
	}

	for (MShaderSampleParam& param : pMaterial->GetPixelShader()->GetBuffer()->m_vSampleParamsTemplate)
	{
		m_pD3dContext->PSSetSamplers(param.unBindPoint, param.unBindCount, &m_pDefaultSamplerState);
	}

}

void MDirectX11Renderer::DrawNode(MNode* pNode, const Matrix4& m4CameraInv)
{
	if (nullptr == pNode)
		return;

	if (false == pNode->GetVisible())
		return;

	MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pNode);
	if (pMeshIns)
	{
		MMaterial* pMaterial = pMeshIns->GetMaterial();
		

		std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vParams)
		{
			if (param.strName == "cbSpace")
			{
				Matrix4 worldTrans = pMeshIns->GetWorldTransform();

				//Transposed and Inverse. but hlsl has been transporsed.
				Matrix3 matNormal(worldTrans.Inverse(), 3, 3);

				MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
				pSpaceStruct->SetMember("MatWorld", worldTrans.Transposed());
				pSpaceStruct->SetMember("MatCamProj", m4CameraInv.Transposed());

				pSpaceStruct->SetMember("MatNormal", matNormal);
				
				break;
			}
		}

		SetUseMaterial(pMaterial);
		DrawMesh(pMeshIns->GetMesh(), m4CameraInv, pMeshIns->GetWorldTransform());
	}

	for (MNode* pChild : pNode->GetChildren())
		DrawNode(pChild, m4CameraInv);
}

void MDirectX11Renderer::DrawMesh(MIMesh* pMesh, const Matrix4& m4CameraInv, const Matrix4& m4ParentMat)
{
	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(this);

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(this);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		UINT stride = pMesh->GetVertexStructSize();
		UINT offset = 0;
		m_pD3dContext->IASetVertexBuffers(0, 1, &pBuffer->m_pVertexBuffer, &stride, &offset);

		m_pD3dContext->IASetIndexBuffer(pBuffer->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		m_pD3dContext->DrawIndexed(pMesh->GetIndicesLength(), 0, 0);
	}
}

void MDirectX11Renderer::CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType)
{
	if (*ppShaderBuffer)
	{
		CleanShader(ppShaderBuffer);
	}

	UINT shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* pErrorMessage = nullptr;
	ID3D10Blob* pShaderBuffer = nullptr;

	const char* svFuncName = eShaderType == MShader::MEShaderType::Vertex ? "VS" : "PS";
	const char* svProFile = eShaderType == MShader::MEShaderType::Vertex ? "vs_5_0" : "ps_5_0";

	HRESULT hr = D3DX11CompileFromFile(strShaderPath.c_str(), NULL, NULL, svFuncName, svProFile, shaderFlags, 0, nullptr, &pShaderBuffer, &pErrorMessage, nullptr);
	if (FAILED(hr))
	{
		if (pErrorMessage)
			MLogManager::GetInstance()->Error("Compile Shader Error: %s", pErrorMessage->GetBufferPointer());
		else
			MLogManager::GetInstance()->Error("Compile Shader Error: Can`t find file: %s", strShaderPath.c_str());

		return;
	}

	if (eShaderType == MShader::MEShaderType::Vertex)
	{
		ID3D11VertexShader* pVertexShader = nullptr;
		hr = m_pD3dDevice->CreateVertexShader(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), nullptr, &pVertexShader);
		if (FAILED(hr))
		{
			MLogManager::GetInstance()->Error("VertexShader is Error!");
			return;
		}

		MVertexShaderBuffer* pBuffer = new MVertexShaderBuffer();
		pBuffer->m_pVertexShader = pVertexShader;
		*ppShaderBuffer = pBuffer;

	}
	else
	{
		ID3D11PixelShader* pPixelShader = nullptr;
		hr = m_pD3dDevice->CreatePixelShader(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), nullptr, &pPixelShader);
		if (FAILED(hr))
		{
			MLogManager::GetInstance()->Error("PixelShader is Error!");
			return;
		}

		MPixelShaderBuffer* pBuffer = new MPixelShaderBuffer();
		pBuffer->m_pPixelShader = pPixelShader;
		*ppShaderBuffer = pBuffer;
	}

	if (*ppShaderBuffer)
	{

		ID3D11ShaderReflection* pReflector = nullptr;
		D3DReflect(pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);

		D3D11_SHADER_DESC shaderDesc;
		pReflector->GetDesc(&shaderDesc);


		if (eShaderType == MShader::MEShaderType::Vertex)
		{
			unsigned int unByteOffset = 0;
			D3D11_INPUT_ELEMENT_DESC* inputDesc = new D3D11_INPUT_ELEMENT_DESC[shaderDesc.InputParameters];
			for (unsigned int i = 0; i < shaderDesc.InputParameters; ++i)
			{
				D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
				pReflector->GetInputParameterDesc(i, &paramDesc);

				D3D11_INPUT_ELEMENT_DESC& inputElementDesc = inputDesc[i];
				inputElementDesc.SemanticName = paramDesc.SemanticName;
				inputElementDesc.SemanticIndex = paramDesc.SemanticIndex;
				inputElementDesc.InputSlot = 0;
				inputElementDesc.AlignedByteOffset = unByteOffset;
				inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				inputElementDesc.InstanceDataStepRate = 0;

				if (paramDesc.Mask == 1){
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32_UINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32_SINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32){
						inputElementDesc.Format = DXGI_FORMAT_R32_FLOAT;
					}
					unByteOffset += 4;
				}
				else if (paramDesc.Mask <= 3){
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32_UINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32_SINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
					}
					unByteOffset += 8;
				}
				else if (paramDesc.Mask <= 7){
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					}
					unByteOffset += 12;
				}
				else if (paramDesc.Mask <= 15){
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					}
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32){
						inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					}
					unByteOffset += 16;
				}

			}

			if (MVertexShaderBuffer* pVertexShaderBuffer = dynamic_cast<MVertexShaderBuffer*>(*ppShaderBuffer))
			{
				pVertexShaderBuffer->m_pInputLayout = CreateInputLayout(inputDesc, shaderDesc.InputParameters);
			}

		}

		for (unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
		{
			ID3D11ShaderReflectionConstantBuffer* pConstBuffer = pReflector->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC bufferDesc;
			pConstBuffer->GetDesc(&bufferDesc);

			MStruct cbufferStruct;

			for (unsigned int n = 0; n < bufferDesc.Variables; ++n)
			{
				D3D11_SHADER_VARIABLE_DESC varDesc;
				ID3D11ShaderReflectionVariable* pVar = pConstBuffer->GetVariableByIndex(n);
				pVar->GetDesc(&varDesc);
				ID3D11ShaderReflectionType* pType = pVar->GetType();
				D3D11_SHADER_TYPE_DESC typeDesc;
				pType->GetDesc(&typeDesc);
				cbufferStruct.AppendVariable(varDesc.Name, typeDesc.Name);
			}


			MShaderParam param;
			param.strName = bufferDesc.Name;
			param.var = MVariable(cbufferStruct);

			(*ppShaderBuffer)->m_vShaderParamsTemplate.push_back(param);
		}

		for (unsigned int i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			pReflector->GetResourceBindingDesc(i, &bindDesc);

			if (D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER == bindDesc.Type)
			{
				for (MShaderParam& param : (*ppShaderBuffer)->m_vShaderParamsTemplate)
				{
					if (param.strName == bindDesc.Name)
					{
						unsigned int unParamDataSize = param.var.GetSize();
						ID3D11Buffer* pBuffer = nullptr;
						D3D11_BUFFER_DESC bufferDesc;
						bufferDesc.ByteWidth = unParamDataSize % 16 ? ((unParamDataSize / 16) + 1) * 16 : unParamDataSize;
						bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
						bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
						bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
						bufferDesc.MiscFlags = 0;
						bufferDesc.StructureByteStride = 0;

						D3D11_SUBRESOURCE_DATA sourceData;
						sourceData.pSysMem = param.var.GetData();
						sourceData.SysMemPitch = 0;
						sourceData.SysMemSlicePitch = 0;

						m_pD3dDevice->CreateBuffer(&bufferDesc, &sourceData, &pBuffer);

						param.pBuffer = pBuffer;
						param.unBindPoint = bindDesc.BindPoint;
						param.unBindCount = bindDesc.BindCount;
						
						break;
					}
				}
			}
			else if (D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE == bindDesc.Type)
			{
				MShaderTextureParam param;
				param.strName = bindDesc.Name;
				param.pTexture = nullptr;
				param.unBindPoint = bindDesc.BindPoint;
				param.unBindCount = bindDesc.BindCount;
				(*ppShaderBuffer)->m_vTextureParamsTemplate.push_back(param);
			}
			else if (D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER == bindDesc.Type)
			{
				MShaderSampleParam param;
				param.unBindPoint = bindDesc.BindPoint;
				param.unBindCount = bindDesc.BindCount;

				(*ppShaderBuffer)->m_vSampleParamsTemplate.push_back(param);
			}
		}

	}

	pShaderBuffer->Release();
	pShaderBuffer = nullptr;



}

void MDirectX11Renderer::CleanShader(MShaderBuffer** ppShaderBuffer)
{
	if (MVertexShaderBuffer* pBuffer = dynamic_cast<MVertexShaderBuffer*>(*ppShaderBuffer))
	{
		if (pBuffer->m_pVertexShader)
		{
			pBuffer->m_pVertexShader->Release();
			pBuffer->m_pVertexShader = nullptr;
		}
	}
	else if (MPixelShaderBuffer* pBuffer = dynamic_cast<MPixelShaderBuffer*>(*ppShaderBuffer))
	{
		if (pBuffer->m_pPixelShader)
		{
			pBuffer->m_pPixelShader->Release();
			pBuffer->m_pPixelShader = nullptr;
		}
	}

	delete *ppShaderBuffer;
	*ppShaderBuffer = nullptr;
}

void MDirectX11Renderer::UpdateShaderParam(MShaderParam& param)
{
	if (nullptr == param.pBuffer)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//  Disable GPU access to the vertex buffer data.
	m_pD3dContext->Map(param.pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//  Update the vertex buffer here.

	memcpy(mappedResource.pData, param.var.GetData(), param.var.GetSize());
	//  Reenable GPU access to the vertex buffer data.
	m_pD3dContext->Unmap(param.pBuffer, 0);
}

