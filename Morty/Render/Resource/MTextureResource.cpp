#include "Resource/MTextureResource.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "spot.hpp"
#include "stb_image.h"

MORTY_CLASS_IMPLEMENT(MTextureResource, MResource)

template <typename ByteType>
MByte* Malloc(const size_t& nSize)
{
	return new MByte[nSize * sizeof(ByteType)];
}


MTextureResource::MTextureResource()
	: MResource()
	, m_texture()
	, m_aByteData(nullptr)
{
	m_texture.SetMipmapsEnable(true);
	m_texture.SetReadable(false);
	m_texture.SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_texture.SetRenderUsage(METextureRenderUsage::EUnknow);
	m_texture.SetShaderUsage(METextureShaderUsage::ESampler);
}

MTextureResource::~MTextureResource()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_texture.DestroyBuffer(pRenderSystem->GetDevice());
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
	m_texture.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTextureResource::LoadFromMemory(MByte* aByteData, const uint32_t& unWidth, const uint32_t& unHeight, uint32_t nChannel, PixelFormat ePixelFormat/* = PixelFormat::Byte8 */, bool bCopyMemory/* = true*/)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_texture.DestroyBuffer(pRenderSystem->GetDevice());

	m_texture.SetSize(Vector2(unWidth, unHeight));

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

		if (!bCopyMemory)
		{
			delete[] aByteData;
		}
	}

	else if (bCopyMemory)
	{
		if (PixelFormat::Byte8 == ePixelFormat)
			m_aByteData = Malloc<MByte>(nSize);
		else if (PixelFormat::Float32 == ePixelFormat)
			m_aByteData = Malloc<float>(nSize);
		memcpy(m_aByteData, aByteData, nSize);
	}
	else
	{
		m_aByteData = aByteData;
	}

	m_texture.SetName(m_strResourcePath);
	m_texture.SetTextureLayout(GetTextureLayout(nChannel, ePixelFormat));
	m_texture.GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);
}

void MTextureResource::CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (nChannel == 2 || nChannel == 3)
		nChannel = 4;

	m_texture.SetName("CubeMapRenderTarget");
	m_texture.SetReadable(true);
	m_texture.SetTextureLayout(eLayout);
	m_texture.SetSize(Vector2(nWidth, nHeight));
	m_texture.SetRenderUsage(METextureRenderUsage::ERenderBack);
	m_texture.SetShaderUsage(METextureShaderUsage::ESampler);
	m_texture.SetTextureType(METextureType::ETextureCube);
	m_texture.SetImageLayerNum(6);
	m_texture.SetMipmapsEnable(bMipmapEnable);

	m_texture.GenerateBuffer(pRenderSystem->GetDevice());
}

bool MTextureResource::ImportTextureFromMemory(char* buffer, size_t nSize, const ImportInfo& importInfo)
{
	int unWidth = 0;
	int unHeight = 0;
	int comp;

	if (importInfo.ePixelFormat == PixelFormat::Byte8)
	{
		stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, 0);
		LoadFromMemory((MByte*)data, unWidth, unHeight, comp, importInfo.ePixelFormat, false);
		data = nullptr;
	}
	else if (importInfo.ePixelFormat == PixelFormat::Float32)
	{
		float* data = stbi_loadf_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, 0);
		LoadFromMemory((MByte*)data, unWidth, unHeight, comp, importInfo.ePixelFormat, false);
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
				free(vImageData[i]);
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
	
	m_texture.SetName(vResourcePath[0]);
	m_texture.SetSize(Vector2(unCubeMapWidth, unCubeMapHeight));
	m_texture.SetTextureType(METextureType::ETextureCube);
	m_texture.SetImageLayerNum(6);
	m_texture.SetTextureLayout(GetTextureLayout(unCubeMapChannel, importInfo.ePixelFormat));


	m_texture.GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);
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
