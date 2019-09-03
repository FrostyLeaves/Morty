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

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

MDirectX11Renderer::MDirectX11Renderer()
	: m_pD3dDevice(nullptr)
	, m_pD3dContext(nullptr)
	, m_pVertexInputLayout(nullptr)
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

	//顶点数据Layout
	D3D11_INPUT_ELEMENT_DESC desc[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	ID3D10Blob* pErrorMessage = nullptr;
	ID3D10Blob* pShaderBuffer = nullptr;

	MString strVirtualShader = "struct VertexIn					\n\
							   							   	{															\n\
																									float3 v1 : POSITION;									\n\
																																					float3 v2 : NORMAL;										\n\
																																																			float2 v3 : TEXCOORDS;									\n\
																																																																			float3 v4 : Tangent;									\n\
																																																																																					float3 v5 : BITANGENT;									\n\
																																																																																																								};															\n\
																																																																																																																																											//\n\
																																																																																																																																																																															struct VertexOut											\n\
																																																																																																																																																																																																																				{															\n\
																																																																																																																																																																																																																																																											float4 PosH : SV_POSITION;								\n\
																																																																																																																																																																																																																																																																																																				float Color : COLOR;									\n\
																																																																																																																																																																																																																																																																																																																																														};															\n\
																																																																																																																																																																																																																																																																																																																																																																																																								\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																			VertexOut VS(VertexIn vin)									\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																															{															\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																													VertexOut vout;											\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																													vout.PosH = float4(vin.v1, 1.0f);						\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																															return vout;											\n\
																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																		}";

	hr = D3DX11CompileFromMemory(strVirtualShader.c_str(), strVirtualShader.size(), nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, nullptr, &pShaderBuffer, &pErrorMessage, nullptr);
	if (FAILED(hr))
	{
		if (pErrorMessage)
			MLogManager::GetInstance()->Error("Compile Shader Error: %s", pErrorMessage->GetBufferPointer());
		else
			MLogManager::GetInstance()->Error("Compile Shader Error: Can`t find file: virtual shader");

		return false;
	}

	//TODO 需要根据顶点数据自动创建出假的Shader，来骗过DX11的Shader-InputLayout验证，让它认为Layout合法。
	hr = m_pD3dDevice->CreateInputLayout(desc, 5, pShaderBuffer->GetBufferPointer(), pShaderBuffer->GetBufferSize(), &m_pVertexInputLayout);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create RasterizerState! Initialize return false.");
		if (m_pVertexInputLayout)
		{
			m_pVertexInputLayout->Release();
			m_pVertexInputLayout = nullptr;
		}

		return false;
	}

	m_pD3dContext->IASetInputLayout(m_pVertexInputLayout);


	//三角形解析顶点
	m_pD3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);





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

	Matrix4 camTransInv = IdentityMatrix;
	if(pCamera)
		camTransInv = pCamera->GetWorldTransform().Inverse();
	DrawNode(pRootNode, camTransInv);
	target.pSwapChain->Present(0, 0);



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

	rt.mViewport.Width = nWidth;
	rt.mViewport.Height = nHeight;

}

void MDirectX11Renderer::GenerateBuffer(MVertexBuffer** ppVertexBuffer, MMesh* pMesh)
{
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
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.ByteWidth = sizeof(unsigned int) * pMesh->GetIndicesLength();
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.CPUAccessFlags = 0;
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

	m_pD3dContext->VSSetShader(dynamic_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer())->m_pVertexShader, nullptr, 0);
	m_pD3dContext->PSSetShader(dynamic_cast<MPixelShaderBuffer*>(pPixelShader->GetBuffer())->m_pPixelShader, nullptr, 0);


	for (MShaderParam& param : pMaterial->GetVertexShaderParams())
	{
		UpdateShaderParam(param);
	}

	for (MShaderParam& param : pMaterial->GetPixelShaderParams())
	{
		UpdateShaderParam(param);
	}
}

void MDirectX11Renderer::DrawNode(MNode* pNode, const Matrix4& m4CameraInv)
{
	if (nullptr == pNode)
		return;

	MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pNode);
	if (pMeshIns)
	{
		MMaterial* pMaterial = pMeshIns->GetMaterial();
		SetUseMaterial(pMaterial);

		std::vector<MShaderParam>& vParams = pMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vParams)
		{
			if (param.strName == "cbSpace")
			{
				Matrix4 projMat = Matrix4::MatrixPerspectiveFovLH(45, 640.0 / 480.0, 0.01, 1000);

				Matrix4 worldTrans = pMeshIns->GetWorldTransform();

				MStruct* pSpaceStruct = param.var.GetStruct();
				pSpaceStruct->SetMember("MatMVP", (worldTrans * m4CameraInv * projMat).Transposed());
	
				break;
			}
		}


		DrawMesh(pMeshIns->GetMesh(), m4CameraInv, pMeshIns->GetWorldTransform());
	}

	for (MNode* pChild : pNode->GetChildren())
		DrawNode(pChild, m4CameraInv);
}

void MDirectX11Renderer::DrawMesh(MMesh* pMesh, const Matrix4& m4CameraInv, const Matrix4& m4ParentMat)
{
	if (nullptr == pMesh->GetBuffer())
		pMesh->GenerateBuffer(this);

	MVertexBuffer* pBuffer = pMesh->GetBuffer();

	UINT stride = sizeof(MVertex);
	UINT offset = 0;
	m_pD3dContext->IASetVertexBuffers(0, 1, &pBuffer->m_pVertexBuffer, &stride, &offset);

	m_pD3dContext->IASetIndexBuffer(pBuffer->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pD3dContext->DrawIndexed(pMesh->GetIndicesLength(), 0, 0);

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

		for (int i = 0; i < shaderDesc.ConstantBuffers; ++i)
		{
			ID3D11ShaderReflectionConstantBuffer* pConstBuffer = pReflector->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC bufferDesc;
			pConstBuffer->GetDesc(&bufferDesc);

			MStruct cbufferStruct;

			for (int n = 0; n < bufferDesc.Variables; ++n)
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

		for (int i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			pReflector->GetResourceBindingDesc(i, &bindDesc);

			for (MShaderParam& param : (*ppShaderBuffer)->m_vShaderParamsTemplate)
			{
				if (param.strName == bindDesc.Name)
				{
					ID3D11Buffer* pBuffer = nullptr;
					D3D11_BUFFER_DESC bufferDesc;
					bufferDesc.ByteWidth = param.var.GetSize();
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
					m_pD3dContext->VSSetConstantBuffers(bindDesc.BindPoint, bindDesc.BindCount, &pBuffer);

					param.pBuffer = pBuffer;

					break;
				}
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

