#include "Resource/MTextureResource.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "stb_image.h"

MORTY_CLASS_IMPLEMENT(MTextureResource, MResource)

template <typename ByteType>
MByte* Malloc(const size_t& nSize)
{
	return new MByte[nSize * sizeof(ByteType)];
}


MTextureResource::MTextureResource()
	: MResource()
	, m_pTexture(std::make_shared<MTexture>())
	, m_aByteData(nullptr)
{
	m_pTexture->SetMipmapsEnable(true);
	m_pTexture->SetReadable(false);
	m_pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_pTexture->SetRenderUsage(METextureRenderUsage::EUnknow);
	m_pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
}

MTextureResource::~MTextureResource()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_pTexture->DestroyBuffer(pRenderSystem->GetDevice());
}

template <typename T>
void FillChannelNum(MByte* aByteData, T* aTargetData, const uint32_t& nSourceSize, const uint32_t& nSourceChannel, const uint32_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{
	T* aSource = (T*)aByteData;

	int nTargetIdx = 0;

	for (int nSourceIdx = 0; nSourceIdx < nSourceSize; nSourceIdx += nSourceChannel)
	{
		for (int nOffset = 0; nOffset < nSourceChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aSource[nSourceIdx + nOffset];
		}

		for (int nOffset = nSourceChannel; nOffset < nTargetChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aDefaultColor[nOffset];
		}
	}
}

template <typename T>
MByte* FillChannelNum(MByte* aByteData, const uint32_t& nSourceSize, const uint32_t& nSourceChannel, const uint32_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{

	uint32_t nNewSize = nSourceSize / nSourceChannel * nTargetChannel;
	T* aTargetData = (T*)(new T[nNewSize]);

	FillChannelNum(aByteData, aTargetData, nSourceSize, nSourceChannel, nTargetChannel, aDefaultColor);

	return (MByte*)(aTargetData);
}

void MTextureResource::OnDelete()
{
	MResource::OnDelete();

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}


	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_pTexture->DestroyBuffer(pRenderSystem->GetDevice());
}

void MTextureResource::LoadFromMemory(MByte* aByteData, const uint32_t& unWidth, const uint32_t& unHeight, uint32_t nChannel, PixelFormat ePixelFormat/* = PixelFormat::Byte8 */)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pTexture->DestroyBuffer(pRenderSystem->GetDevice());

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}

	const size_t nSize = unWidth * unHeight * nChannel;

	
	if (nChannel == 2 || nChannel == 3)
	{
		if (PixelFormat::Byte8 == ePixelFormat)
			m_aByteData = FillChannelNum<MByte>(aByteData, nSize, nChannel, 4, { 0, 0, 0, 255 });
		else if (PixelFormat::Float32 == ePixelFormat)
			m_aByteData = FillChannelNum<float>(aByteData, nSize, nChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f });

		nChannel = 4;
	}
	else
	{
		if (PixelFormat::Byte8 == ePixelFormat)
			m_aByteData = Malloc<MByte>(nSize);
		else if (PixelFormat::Float32 == ePixelFormat)
			m_aByteData = Malloc<float>(nSize);
		memcpy(m_aByteData, aByteData, nSize);
	}

	m_pTexture->SetName(m_strResourcePath);
	m_pTexture->SetTextureLayout(GetTextureLayout(nChannel, ePixelFormat));
	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}
}

void MTextureResource::CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (nChannel == 2 || nChannel == 3)
		nChannel = 4;

	m_pTexture->SetName("CubeMapRenderTarget");
	m_pTexture->SetReadable(true);
	m_pTexture->SetTextureLayout(eLayout);
	m_pTexture->SetSize(Vector2(nWidth, nHeight));
	m_pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	m_pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	m_pTexture->SetTextureType(METextureType::ETextureCube);
	m_pTexture->SetImageLayerNum(6);
	m_pTexture->SetMipmapsEnable(bMipmapEnable);

	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice());
}

bool MTextureResource::ImportTextureFromMemory(char* buffer, size_t nSize, const ImportInfo& importInfo)
{
	int unWidth = 0;
	int unHeight = 0;
	int comp = 4;
	int reqComp = 4;

	if (importInfo.ePixelFormat == PixelFormat::Byte8)
	{
		stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, reqComp);
		LoadFromMemory((MByte*)data, unWidth, unHeight, reqComp, importInfo.ePixelFormat);
		stbi_image_free(data);
		data = nullptr;
	}
	else if (importInfo.ePixelFormat == PixelFormat::Float32)
	{
		float* data = stbi_loadf_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, reqComp);
		LoadFromMemory((MByte*)data, unWidth, unHeight, reqComp, importInfo.ePixelFormat);
		stbi_image_free(data);
		data = nullptr;
	}
	else
	{
		GetEngine()->GetLogger()->Error("Load Texture Error: unknow format.");
		return false;
	}

	return true;
}

bool MTextureResource::ImportTexture(const MString& strResourcePath, const ImportInfo& importInfo)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
		return false;
	}

	std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	return ImportTextureFromMemory(buffer.data(), buffer.size(), importInfo);
}

bool MTextureResource::ImportCubeMap(const std::array<MString, 6>& vResourcePath, const ImportInfo& importInfo)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (!pResourceSystem || !pRenderSystem)
		return false;

	int unCubeMapWidth = 0;
	int unCubeMapHeight = 0;
	int unCubeMapChannel = 0;

	std::array<MByte*, 6> vImageData = {};


	bool bError = false;
	for (int fileIdx = 0; fileIdx < 6; ++fileIdx)
	{
		const MString& strResourcePath = vResourcePath[fileIdx];

		MString strFullPath = pResourceSystem->GetFullPath(strResourcePath);

		std::ifstream ifs(strFullPath.c_str(), std::ios::binary);
		if (!ifs.good())
		{
			bError = true;
			break;
		}

		int unWidth = 0;
		int unHeight = 0;
		int unChannel = 0;

		std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		if (importInfo.ePixelFormat == PixelFormat::Byte8)
		{
			stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &unChannel, 0);
			vImageData[fileIdx] = (MByte*)data;
		}
		else if (importInfo.ePixelFormat == PixelFormat::Float32)
		{
			float* data = stbi_loadf_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &unChannel, 0);
			vImageData[fileIdx] = (MByte*)data;
		}
		else
		{
			GetEngine()->GetLogger()->Error("Load Texture Error: unknow format.");
			bError = true;
			break;
		}

		if (0 == fileIdx)
		{
			unCubeMapWidth = unWidth;
			unCubeMapHeight = unHeight;
			unCubeMapChannel = unChannel;
		}
		else if (unCubeMapWidth != unWidth || unCubeMapHeight != unHeight || unCubeMapChannel != unChannel)
		{
			bError = true;
			break;
		}
	}

	if (bError)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (vImageData[i])
			{
				stbi_image_free(vImageData[i]);
				vImageData[i] = nullptr;
			}
		}

		return false;
	}

	if (unCubeMapChannel == 1 || unCubeMapChannel == 4)
	{
		int nTextureSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;

		if (PixelFormat::Byte8 == importInfo.ePixelFormat)
		{
			m_aByteData = Malloc<MByte>(nTextureSize * 6);
		}
		else if (PixelFormat::Float32 == importInfo.ePixelFormat)
		{
			m_aByteData = Malloc<float>(nTextureSize * 6);
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			memcpy(m_aByteData + nTextureSize * nTexIdx, vImageData[nTexIdx], nTextureSize);
		}
	}
	else
	{
		int nSourceSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;
		int nTargetSize = unCubeMapWidth * unCubeMapHeight * 4;

		if (PixelFormat::Byte8 == importInfo.ePixelFormat)
		{
			m_aByteData = Malloc<MByte>(nTargetSize * 6 );
		}
		else if (PixelFormat::Float32 == importInfo.ePixelFormat)
		{
			m_aByteData = Malloc<float>(nTargetSize * 6);
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			if (PixelFormat::Byte8 == importInfo.ePixelFormat)
			{
				FillChannelNum<MByte>(vImageData[nTexIdx], (MByte*)(m_aByteData) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0, 0, 0, 255 });
			}
			else if (PixelFormat::Float32 == importInfo.ePixelFormat)
			{
				FillChannelNum<float>(vImageData[nTexIdx], (float*)(m_aByteData) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f});
			}
		}

		unCubeMapChannel = 4;
	}


	for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
	{
		free(vImageData[nTexIdx]);
		vImageData[nTexIdx] = nullptr;
	}
	
	m_pTexture->SetName(vResourcePath[0]);
	m_pTexture->SetSize(Vector2(unCubeMapWidth, unCubeMapHeight));
	m_pTexture->SetTextureType(METextureType::ETextureCube);
	m_pTexture->SetImageLayerNum(6);
	m_pTexture->SetTextureLayout(GetTextureLayout(unCubeMapChannel, importInfo.ePixelFormat));


	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}

	return true;
}

METextureLayout MTextureResource::GetTextureLayout(const uint32_t& nChannel, const PixelFormat& format)
{
	METextureLayout eResult = METextureLayout::E_UNKNOW;
	if (PixelFormat::Byte8 == format)
	{
		static const std::array<METextureLayout, 4> sTextureLayout = {
			METextureLayout::ER_UNORM_8,
			METextureLayout::E_UNKNOW,
			METextureLayout::ERGB_UNORM_8,
			METextureLayout::ERGBA_UNORM_8,
		};

		if (0 <= nChannel && nChannel <= 4)
		{
			eResult = sTextureLayout[nChannel - 1];
		}
	}
	else if (PixelFormat::Float32 == format)
	{
		static const std::array<METextureLayout, 4> sTextureLayout = {
			METextureLayout::ER_FLOAT_32,
			METextureLayout::E_UNKNOW,
			METextureLayout::E_UNKNOW,
			METextureLayout::ERGBA_FLOAT_32,
		};

		if (0 <= nChannel && nChannel <= 4)
		{
			eResult = sTextureLayout[nChannel - 1];
		}
	}

	MORTY_ASSERT(METextureLayout::E_UNKNOW != eResult);

	return eResult;
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	ImportInfo importInfo;
	importInfo.ePixelFormat = PixelFormat::Byte8;

	return ImportTexture(strResourcePath, importInfo);
}

bool MTextureResource::SaveTo(const MString& strResourcePath)
{
	return false;
}

MTextureResource::ImportInfo::ImportInfo()
	: ePixelFormat(PixelFormat::Byte8)
{
}

MTextureResource::ImportInfo::ImportInfo(PixelFormat pixelFormat)
	: ePixelFormat(pixelFormat)
{
}
