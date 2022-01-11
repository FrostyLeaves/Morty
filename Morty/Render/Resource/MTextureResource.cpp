#include "MTextureResource.h"

#include "MEngine.h"
#include "MIDevice.h"

#include "MRenderSystem.h"

#include "spot.hpp"
#include "stb_image.h"

MORTY_CLASS_IMPLEMENT(MTextureResource, MResource)

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

void MTextureResource::LoadFromMemory(MByte* aByteData, const uint32_t& unWidth, const uint32_t& unHeight, const uint32_t& nChannel, PixelFormat ePixelFormat/* = PixelFormat::Byte8 */, bool bCopyMemory/* = true*/)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_texture.DestroyBuffer(pRenderSystem->GetDevice());

	m_texture.SetSize(Vector2(unWidth, unHeight));

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}

	if (PixelFormat::Byte8 == ePixelFormat)
	{
		const size_t nSize = unWidth * unHeight * nChannel;

		if (bCopyMemory)
		{
			m_aByteData = new MByte[nSize];
			memcpy(m_aByteData, aByteData, nSize);
		}
		else
		{
			m_aByteData = aByteData;
		}
		m_texture.SetTextureLayout(GetTextureLayout(nChannel, ePixelFormat));
	}
	else if (PixelFormat::Float32 == ePixelFormat)
	{
		const size_t nSize = unWidth * unHeight * nChannel * 4;
		m_aByteData = new MByte[nSize];
		memcpy(m_aByteData, aByteData, nSize);
		m_texture.SetTextureLayout(GetTextureLayout(nChannel, ePixelFormat));
	}

	m_texture.GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);
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

	int unWidth = 0;
	int unHeight = 0;
	int comp;

	if (importInfo.ePixelFormat == PixelFormat::Byte8)
	{
		stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &comp, 0);
		LoadFromMemory((MByte*)data, unWidth, unHeight, comp, importInfo.ePixelFormat, false);
		data = nullptr;
	}
	else if (importInfo.ePixelFormat == PixelFormat::Float32)
	{
		float* data = stbi_loadf_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &comp, 0);
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

bool MTextureResource::ImportCubeMap(const std::array<MString, 6>& vResourcePath, const ImportInfo& importInfo)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	int unCubeMapWidth = 0;
	int unCubeMapHeight = 0;
	int unCubeMapChannel = 0;

	std::array<MByte*, 6> vImageData = {};


	bool bError = false;
	for (int fileIdx = 0; fileIdx < 6; ++fileIdx)
	{
		const MString& strResourcePath = vResourcePath[fileIdx];

		std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
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

	int nTextureSize = 0;
	if (PixelFormat::Byte8 == importInfo.ePixelFormat)
	{
		nTextureSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;
		m_aByteData = new MByte[nTextureSize * 6];
		m_texture.SetTextureLayout(GetTextureLayout(unCubeMapChannel, PixelFormat::Byte8));
	}
	else if (PixelFormat::Float32 == importInfo.ePixelFormat)
	{
		nTextureSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel * 4;
		m_aByteData = new MByte[nTextureSize * 6];
		m_texture.SetTextureLayout(GetTextureLayout(unCubeMapChannel, PixelFormat::Float32));
	}

	m_texture.SetSize(Vector2(unCubeMapWidth, unCubeMapHeight));
	m_texture.SetTextureType(METextureType::ETextureCube);

	for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
	{
		memcpy(m_aByteData + nTextureSize * nTexIdx, vImageData[nTexIdx], nTextureSize);
		free(vImageData[nTexIdx]);
		vImageData[nTexIdx] = nullptr;
	}

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

	assert(METextureLayout::E_UNKNOW != eResult);

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
