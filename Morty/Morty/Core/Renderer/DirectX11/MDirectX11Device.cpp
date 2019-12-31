#include "MDirectX11Device.h"

#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <d3d11shader.h>
#include <DxErr.h>

#include "MLogManager.h"
#include "MMesh.h"
#include "MVertex.h"
#include "MRenderStructure.h"
#include "MTexture.h"
#include "MShader.h"
#include "MIRenderTarget.h"

const bool bEnable4xMsaa = true;


MDirectX11Device::MDirectX11Device()
	: MIDevice()
	, m_pD3dDevice(nullptr)
	, m_pD3dContext(nullptr)
	, m_n4xMsaaQuality(0)
	, m_nDriverType(D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_NULL)
	, m_nFeatureLevel(D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0)
{

}

MDirectX11Device::~MDirectX11Device()
{

}

bool MDirectX11Device::InitDirectX11()
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

			if (m_n4xMsaaQuality > 0)
			{
				//we can set quality is the number that less then the value but can`t equal it.
				m_n4xMsaaQuality = m_n4xMsaaQuality - 1;
			}
			return true;
		}
		else
		{
		}
	}

	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to create the Direct3D device!");
		return false;
	}

	return true;
}

bool MDirectX11Device::Initialize()
{
	if (nullptr == m_pD3dDevice && false == InitDirectX11())
		return false;

	return true;
}

void MDirectX11Device::Release()
{
	if (m_pD3dContext)
	{
		m_pD3dContext->Release();
		m_pD3dContext = nullptr;
	}

// #if defined(DEBUG) || defined(_DEBUG)
// 	ID3D11Debug *d3dDebug;
// 	HRESULT hr = m_pD3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
// 	if (SUCCEEDED(hr))
// 	{
// 		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
// 	}
// 	if (d3dDebug != nullptr)
// 	{
// 		d3dDebug->Release();
// 		d3dDebug = nullptr;
// 	}
// #endif

	if (m_pD3dDevice)
	{
		m_pD3dDevice->Release();
		m_pD3dDevice = nullptr;
	}
}

void MDirectX11Device::GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable/* = false*/)
{
	if (pMesh->GetIndicesLength() <= 0)
	{
		ppVertexBuffer = nullptr;
		return;
	}

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

void MDirectX11Device::GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGenerateMipmap)
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

		(*ppTextureBuffer)->m_pTextureBuffer = pTextureBuffer;
		(*ppTextureBuffer)->m_pShaderResourceView = pShaderResourceView;
	}
}

void MDirectX11Device::GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap)
{
	if (nullptr == vTexture)
		return;

	for (int i = 0; i < 6; ++i)
	{
		if (nullptr == vTexture[i])
			return;
	}

	Vector2 size = vTexture[0]->GetSize();
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = (unsigned int)size.x;
	desc.Height = (unsigned int)size.y;
	desc.MipLevels = bGenerateMipmap ? 0 : 1;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_FLAG::D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.MipLevels = bGenerateMipmap ? -1 : 1;

	ID3D11Texture2D* pTextureBuffer = nullptr;

	if (bGenerateMipmap)
	{
		m_pD3dDevice->CreateTexture2D(&desc, nullptr, &pTextureBuffer);
		pTextureBuffer->GetDesc(&desc);

		for (int i = 0; i < 6; ++i)
		{
			m_pD3dContext->UpdateSubresource(pTextureBuffer, i, nullptr, vTexture[i]->GetImageData(), (unsigned int)size.x * 4, 0);
		}
	}
	else
	{
		D3D11_SUBRESOURCE_DATA data[6];
		for (int i = 0; i < 6; ++i)
		{
			data[i].pSysMem = vTexture[i]->GetImageData();
			data[i].SysMemPitch = (unsigned int)size.x * 4;
			data[i].SysMemSlicePitch = (unsigned int)size.x * size.y * 4;
		}

		HRESULT hr = m_pD3dDevice->CreateTexture2D(&desc, data, &pTextureBuffer);
		if (HRESULT(hr))
		{
			return;
		}
	}

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
		
		(*ppTextureBuffer)->m_pTextureBuffer = pTextureBuffer;
		(*ppTextureBuffer)->m_pShaderResourceView = pShaderResourceView;
	}
}

void MDirectX11Device::DestroyBuffer(MVertexBuffer** ppVertexBuffer)
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

void MDirectX11Device::DestroyTexture(MTextureBuffer** ppTextureBuffer)
{
	if ((*ppTextureBuffer)->m_pTextureBuffer)
	{
		(*ppTextureBuffer)->m_pTextureBuffer->Release();
		(*ppTextureBuffer)->m_pTextureBuffer = nullptr;
	}
	if ((*ppTextureBuffer)->m_pShaderResourceView)
	{
		(*ppTextureBuffer)->m_pShaderResourceView->Release();
		(*ppTextureBuffer)->m_pShaderResourceView = nullptr;
	}

	delete *ppTextureBuffer;
	*ppTextureBuffer = nullptr;
}

void MDirectX11Device::UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh)
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

void MDirectX11Device::CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType)
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


	const MString strBonesPerVertex = MStringHelper::ToString(MBONES_PER_VERTEX);
	const MString strBonesMaxNumber = MStringHelper::ToString(MBONES_MAX_NUMBER);
	D3D_SHADER_MACRO macro[] = {
		"MBONES_PER_VERTEX", strBonesPerVertex.c_str(),
		"MBONES_MAX_NUMBER", strBonesMaxNumber.c_str(),
		nullptr, nullptr
		};
	HRESULT hr = D3DX11CompileFromFile(strShaderPath.c_str(), macro, nullptr, svFuncName, svProFile, shaderFlags, 0, nullptr, &pShaderBuffer, &pErrorMessage, nullptr);
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
				cbufferStruct.AppendMVariant(varDesc.Name, GenerateVariableByBuffer(pType));
			}


			MShaderParam param;
			param.strName = bufferDesc.Name;
			param.var = MVariant(cbufferStruct);

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
				
				if (D3D11_SRV_DIMENSION_TEXTURECUBE == bindDesc.Dimension)
					param.eType = ETextureCube;
				else
					param.eType = ETexture2D;

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

void MDirectX11Device::CleanShader(MShaderBuffer** ppShaderBuffer)
{
	if (nullptr == *ppShaderBuffer)
		return;

	if (MVertexShaderBuffer* pBuffer = dynamic_cast<MVertexShaderBuffer*>(*ppShaderBuffer))
	{
		if (pBuffer->m_pVertexShader)
		{
			pBuffer->m_pVertexShader->Release();
			pBuffer->m_pVertexShader = nullptr;
		}
		if (pBuffer->m_pInputLayout)
		{
			pBuffer->m_pInputLayout->Release();
			pBuffer->m_pInputLayout = nullptr;
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

	for (MShaderParam& param : (*ppShaderBuffer)->m_vShaderParamsTemplate)
	{
		param.pBuffer->Release();
		param.pBuffer = nullptr;
	}

	delete *ppShaderBuffer;
	*ppShaderBuffer = nullptr;
}

ID3D11InputLayout* MDirectX11Device::CreateInputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const int& nLength)
{
	ID3D10Blob* pErrorMessage = nullptr;
	ID3D10Blob* pShaderBuffer = nullptr;

	const MString strVirtualShaderFront = "struct VertexIn {	\n\ ";
	const MString strVirtualShaderBack = "};	\n struct VertexOut	\n {	\n float4 PosH : SV_POSITION;\n float Color : COLOR;	\n };	\n VertexOut VS(VertexIn vin)	\n {	\n VertexOut vout;	\n vout.PosH = float4(1.0f, 1.0f, 1.0f, 1.0f);\n return vout;	\n }";

	MString strVirtualShader = strVirtualShaderFront;
	for (int i = 0; i < nLength; ++i)
	{
		MString strType;
		if (i < nLength - 1 && MString(desc[i].SemanticName) == MString(desc[i + 1].SemanticName))
		{
			continue;
		}
		switch (desc[i].Format)
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			strType = "float4";
			break;
		case DXGI_FORMAT_R32G32B32_FLOAT:
			strType = "float3";
			break;
		case DXGI_FORMAT_R32G32_FLOAT:
			strType = "float2";
			break;
		case DXGI_FORMAT_R32_SINT:
			strType = "int";
			break;
		default:
			strType = "float";
			break;
		}

		strVirtualShader += strType + "    v_" + MStringHelper::ToString(i);
		if (desc[i].SemanticIndex > 0)
			strVirtualShader += "[" + MStringHelper::ToString(desc[i].SemanticIndex + 1) + "]";
		strVirtualShader += MString(" : ") + desc[i].SemanticName + " ; \n";
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

bool MDirectX11Device::GenerateRenderTarget(MIRenderTarget* pRenderTarget, int nWidth, int nHeight)
{
	if (nullptr == pRenderTarget->m_pBackBuffer)
		return false;

	if (nWidth < 1)
		nWidth = 1;
	if (nHeight < 1)
		nHeight = 1;

	// Resize the swap chain and recreate the render target view.
	HRESULT hr;

	hr = m_pD3dDevice->CreateRenderTargetView(pRenderTarget->m_pBackBuffer, 0, &pRenderTarget->m_pTargetView);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateRenderTargetView!");
		return false;
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

	hr = m_pD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &pRenderTarget->m_pDepthStencilBuffer);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateTexture2D!");
		return false;
	}

	hr = m_pD3dDevice->CreateDepthStencilView(pRenderTarget->m_pDepthStencilBuffer, 0, &pRenderTarget->m_pDepthStencilView);
	if (FAILED(hr))
	{
		MLogManager::GetInstance()->Error("Failed to CreateDepthStencilView!");
		return false;
	}

	return true;
}

void MDirectX11Device::DestroyRenderTarget(MIRenderTarget* pRenderTarget)
{
	if (pRenderTarget->m_pBackBuffer)
	{
		pRenderTarget->m_pBackBuffer->Release();
		pRenderTarget->m_pBackBuffer = nullptr;
	}
	if (pRenderTarget->m_pTargetView)
	{
		pRenderTarget->m_pTargetView->Release();
		pRenderTarget->m_pTargetView = nullptr;
	}

	if (pRenderTarget->m_pDepthStencilBuffer)
	{
		pRenderTarget->m_pDepthStencilBuffer->Release();
		pRenderTarget->m_pDepthStencilBuffer = nullptr;
	}

	if (pRenderTarget->m_pDepthStencilView)
	{
		pRenderTarget->m_pDepthStencilView->Release();
		pRenderTarget->m_pDepthStencilView = nullptr;
	}

}

MVariant MDirectX11Device::GenerateVariableByBuffer(ID3D11ShaderReflectionType* pReflectionType)
{
	MVariant var;

	D3D11_SHADER_TYPE_DESC typeDesc;
	pReflectionType->GetDesc(&typeDesc);

	
	if (typeDesc.Members > 0)
	{
		//Is a struct.
		MStruct mryStruct;

		for (unsigned int i = 0; i < typeDesc.Members; ++i)
		{
			ID3D11ShaderReflectionType* pMemberType = pReflectionType->GetMemberTypeByIndex(i);
			MString strName = pReflectionType->GetMemberTypeName(i);

			D3D11_SHADER_TYPE_DESC childTypeDesc;
			pMemberType->GetDesc(&childTypeDesc);

			mryStruct.AppendMVariant(strName, GenerateVariableByBuffer(pMemberType));
		}


		var = mryStruct;
	}
	else
	{
		//Is a MVariant.
		MString type = typeDesc.Name;

		if (type == "bool")
		{
			var = MVariant(false);
		}
		else if (type == "int")
		{
			var = MVariant(0);
		}
		else if (type == "float")
		{
			var = MVariant(0.0f);
		}
		else if (type == "float4")
		{
			var = MVariant(Vector4());
		}
		else if (type == "float3")
		{
			var = MVariant(Vector3());
		}
		else if (type == "float3x3")
		{
			var = MVariant(Matrix3());
		}
		else if (type == "float4x4")
		{
			var = MVariant(Matrix4());
		}
		else
		{
			MLogManager::GetInstance()->Error("GenerateVariableByBuffer: Undefined type :%s", type);
		}

	}

	if (typeDesc.Elements > 0)
	{
		//Is an array.
		MVariantArray array;

		for (unsigned int i = 0; i < typeDesc.Elements; ++i)
			array.AppendMVariant(var);

		return array;
	}

	return var;
}
