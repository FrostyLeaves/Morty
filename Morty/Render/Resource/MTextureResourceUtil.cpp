#include "Resource/MTextureResourceUtil.h"

#include "MTextureResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "stb_image.h"

using namespace morty;

template <typename T>
void FillChannelNum(const MByte* aByteData, T* aTargetData, const size_t& nSourceSize, const size_t& nSourceChannel, const size_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{
	const T* aSource = (const T*)aByteData;

	size_t nTargetIdx = 0;

	for (size_t nSourceIdx = 0; nSourceIdx < nSourceSize; nSourceIdx += nSourceChannel)
	{
		for (size_t nOffset = 0; nOffset < nSourceChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aSource[nSourceIdx + nOffset];
		}

		for (size_t nOffset = nSourceChannel; nOffset < nTargetChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aDefaultColor[nOffset];
		}
	}
}

template <typename T>
std::vector<MByte> FillChannelNum(const MByte* aByteData, const size_t& nSourceSize, const size_t& nSourceChannel, const size_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{

    size_t nNewSize = nSourceSize / nSourceChannel * nTargetChannel;
	std::vector<MByte> aTargetData(nNewSize);

	FillChannelNum(aByteData, reinterpret_cast<T*>(aTargetData.data()), nSourceSize, nSourceChannel, nTargetChannel, aDefaultColor);

	return aTargetData;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::LoadFromMemory(const MString& strTextureName, const MSpan<MByte>& buffer, const uint32_t& unWidth, const uint32_t& unHeight, uint32_t nChannel, MTexturePixelType ePixelType)
{
	std::unique_ptr<MTextureResourceData> textureData = std::make_unique<MTextureResourceData>();

	const size_t nSize = unWidth * unHeight * nChannel;

	MORTY_ASSERT(ePixelType != MTexturePixelType::Unknow);

	if (nChannel == 2 || nChannel == 3)
	{
		if (ePixelType == MTexturePixelType::Byte8)
			textureData->vMipmaps.push_back(FillChannelNum<MByte>(buffer.data(), nSize, nChannel, 4, { 0, 0, 0, 255 }));
		else if (ePixelType == MTexturePixelType::Float32)
			textureData->vMipmaps.push_back(FillChannelNum<float>(buffer.data(), nSize, nChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f }));

		nChannel = 4;
	}
	else
	{
		if (ePixelType == MTexturePixelType::Byte8)
			textureData->vMipmaps.push_back({ buffer.begin(), buffer.begin() + sizeof(MByte) * nSize });
		else if (ePixelType == MTexturePixelType::Float32)
			textureData->vMipmaps.push_back({ buffer.begin(), buffer.begin() + sizeof(float) * nSize });
	}

	textureData->nWidth = unWidth;
	textureData->nHeight = unHeight;
	textureData->eFormat = MTextureResourceUtil::GetTextureFormat(ePixelType, nChannel);
	textureData->strTextureName = strTextureName;

	return textureData;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::ImportTextureFromMemory(const MSpan<MByte>& buffer, const MTextureImportInfo& importInfo)
{
	std::unique_ptr<MResourceData> pTextureData = nullptr;

	int unWidth = 0;
	int unHeight = 0;
	int comp = 4;
	int reqComp = 4;



	if (importInfo.ePixelType == MTexturePixelType::Byte8)
	{
		MByte* data = reinterpret_cast<MByte*>(stbi_load_from_memory((const stbi_uc*)buffer.data(), static_cast<uint32_t>(buffer.size()), &unWidth, &unHeight, &comp, reqComp));
		const size_t nSize = unWidth * unHeight * reqComp * 1;
		pTextureData = LoadFromMemory("ImportFromMemoryTexture", MSpan<MByte>{data, nSize}, unWidth, unHeight, reqComp, importInfo.ePixelType);
		stbi_image_free(data);
		data = nullptr;
	}
	else if (importInfo.ePixelType == MTexturePixelType::Float32)
	{
		MByte* data = reinterpret_cast<MByte*>(stbi_loadf_from_memory((const stbi_uc*)buffer.data(), static_cast<uint32_t>(buffer.size()), &unWidth, &unHeight, &comp, reqComp));
		const size_t nSize = unWidth * unHeight * reqComp * 4;
		pTextureData = LoadFromMemory("ImportFromMemoryTexture", MSpan<MByte>{data, nSize}, unWidth, unHeight, reqComp, importInfo.ePixelType);
		stbi_image_free(data);
		data = nullptr;
	}
    else
    {
		MORTY_ASSERT(false);
    }
	return pTextureData;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::ImportTexture(const MString& strResourcePath, const MTextureImportInfo& importInfo)
{
	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
		return nullptr;
	}

	std::vector<MByte> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	auto pResult = ImportTextureFromMemory(buffer, importInfo);
	if (auto pTextureData = static_cast<MTextureResourceData*>(pResult.get()))
	{
		pTextureData->strTextureName = strResourcePath;
	}

	return pResult;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::ImportCubeMap(const std::array<MString, 6>& vResourcePath, const MTextureImportInfo& importInfo)
{
	std::unique_ptr<MTextureResourceData> textureData = std::make_unique<MTextureResourceData>();

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
			MORTY_ASSERT(ifs.good());
			bError = true;
			break;
		}

		int unWidth = 0;
		int unHeight = 0;
		int unChannel = 0;

		std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
		if (importInfo.ePixelType == MTexturePixelType::Byte8)
		{
			stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer.data(), static_cast<uint32_t>(buffer.size()), &unWidth, &unHeight, &unChannel, 0);
			vImageData[fileIdx] = (MByte*)data;
		}
		else if (importInfo.ePixelType == MTexturePixelType::Float32)
		{
			float* data = stbi_loadf_from_memory((const stbi_uc*)buffer.data(), static_cast<uint32_t>(buffer.size()), &unWidth, &unHeight, &unChannel, 0);
			vImageData[fileIdx] = (MByte*)data;
		}
		else
		{
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

		return nullptr;
	}

	textureData->vMipmaps.push_back({});

	if (unCubeMapChannel == 1 || unCubeMapChannel == 4)
	{
		int nTextureSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;

		if (importInfo.ePixelType == MTexturePixelType::Byte8)
		{
			nTextureSize *= sizeof(MByte);
			textureData->vMipmaps[0].resize(sizeof(MByte) * (nTextureSize * 6));
		}
		else if (importInfo.ePixelType == MTexturePixelType::Float32)
		{
			nTextureSize += sizeof(float);
			textureData->vMipmaps[0].resize(sizeof(float) * (nTextureSize * 6));
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			memcpy(textureData->vMipmaps[0].data() + nTextureSize * nTexIdx, vImageData[nTexIdx], nTextureSize);
		}
	}
	else
	{
		int nSourceSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;
		int nTargetSize = unCubeMapWidth * unCubeMapHeight * 4;

		if (importInfo.ePixelType == MTexturePixelType::Byte8)
		{
			textureData->vMipmaps[0].resize(sizeof(MByte) * (nTargetSize * 6 ));
		}
		else if (importInfo.ePixelType == MTexturePixelType::Float32)
		{
			textureData->vMipmaps[0].resize(sizeof(float) * (nTargetSize * 6));
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			if (importInfo.ePixelType == MTexturePixelType::Byte8)
			{
				FillChannelNum<MByte>(vImageData[nTexIdx], (MByte*)(textureData->vMipmaps[0].data()) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0, 0, 0, 255 });
			}
			else if (importInfo.ePixelType == MTexturePixelType::Float32)
			{
				FillChannelNum<float>(vImageData[nTexIdx], (float*)(textureData->vMipmaps[0].data()) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f});
			}
		}

		unCubeMapChannel = 4;
	}


	for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
	{
		free(vImageData[nTexIdx]);
		vImageData[nTexIdx] = nullptr;
	}

	textureData->nWidth = unCubeMapWidth;
	textureData->nHeight = unCubeMapHeight;
	textureData->nDepth = 6;
	textureData->eFormat = MTextureResourceUtil::GetTextureFormat(importInfo.ePixelType, unCubeMapChannel);
	textureData->strTextureName = vResourcePath[0];
	textureData->eTextureType = METextureType::ETextureCube;
	
	return textureData;
}

morty::METextureLayout MTextureResourceUtil::GetTextureFormat(const MTexturePixelType nPixelSize, const size_t nChannelNum)
{
	if (nPixelSize == MTexturePixelType::Byte8)
	{
	    if (nChannelNum == 1)
	    {
			return morty::METextureLayout::UNorm_R8;
	    }
		if (nChannelNum == 4)
		{
			return morty::METextureLayout::UNorm_RGBA8;
		}
	}

	if (nPixelSize == MTexturePixelType::Float32)
	{
	    if (nChannelNum == 1)
	    {
			return morty::METextureLayout::Float_R32;
	    }
		if (nChannelNum == 4)
		{
			return morty::METextureLayout::Float_RGBA32;
		}
	}


	MORTY_ASSERT(false);
	return morty::METextureLayout::Unknow;
}
