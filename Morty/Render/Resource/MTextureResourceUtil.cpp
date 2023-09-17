#include "Resource/MTextureResourceUtil.h"

#include "MTextureResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "stb_image.h"

template <typename T>
void FillChannelNum(const MByte* aByteData, T* aTargetData, const uint32_t& nSourceSize, const uint32_t& nSourceChannel, const uint32_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{
	const T* aSource = (const T*)aByteData;

	int nTargetIdx = 0;

	for (uint32_t nSourceIdx = 0; nSourceIdx < nSourceSize; nSourceIdx += nSourceChannel)
	{
		for (uint32_t nOffset = 0; nOffset < nSourceChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aSource[nSourceIdx + nOffset];
		}

		for (uint32_t nOffset = nSourceChannel; nOffset < nTargetChannel; ++nOffset)
		{
			aTargetData[nTargetIdx++] = aDefaultColor[nOffset];
		}
	}
}

template <typename T>
std::vector<MByte> FillChannelNum(const MByte* aByteData, const uint32_t& nSourceSize, const uint32_t& nSourceChannel, const uint32_t& nTargetChannel, const std::array<T, 4>& aDefaultColor)
{

	uint32_t nNewSize = nSourceSize / nSourceChannel * nTargetChannel;
	std::vector<MByte> aTargetData(nNewSize);

	FillChannelNum(aByteData, reinterpret_cast<T*>(aTargetData.data()), nSourceSize, nSourceChannel, nTargetChannel, aDefaultColor);

	return aTargetData;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::LoadFromMemory(const MString& strTextureName, const MByte* aByteData, const uint32_t& unWidth, const uint32_t& unHeight, uint32_t nChannel, MTexturePixelFormat ePixelFormat/* = MTexturePixelFormat::Byte8 */)
{
	std::unique_ptr<MTextureResourceData> textureData = std::make_unique<MTextureResourceData>();

	const size_t nSize = unWidth * unHeight * nChannel;

	if (nChannel == 2 || nChannel == 3)
	{
		if (MTexturePixelFormat::Byte8 == ePixelFormat)
			textureData->aByteData = FillChannelNum<MByte>(aByteData, nSize, nChannel, 4, { 0, 0, 0, 255 });
		else if (MTexturePixelFormat::Float32 == ePixelFormat)
			textureData->aByteData = FillChannelNum<float>(aByteData, nSize, nChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f });

		nChannel = 4;
	}
	else
	{
		if (MTexturePixelFormat::Byte8 == ePixelFormat)
			textureData->aByteData.resize(sizeof(MByte) * nSize);
		else if (MTexturePixelFormat::Float32 == ePixelFormat)
			textureData->aByteData.resize(sizeof(float) * nSize);
		memcpy(textureData->aByteData.data(), aByteData, nSize);
	}

	textureData->nWidth = unWidth;
	textureData->nHeight = unHeight;
	textureData->nChannel = nChannel;
	textureData->ePixelFormat = ePixelFormat;
	textureData->strTextureName = strTextureName;

	return textureData;
}

std::unique_ptr<MResourceData> MTextureResourceUtil::ImportTextureFromMemory(char* buffer, size_t nSize, const MTextureImportInfo& importInfo)
{
	std::unique_ptr<MResourceData> pTextureData = nullptr;

	int unWidth = 0;
	int unHeight = 0;
	int comp = 4;
	int reqComp = 4;

	if (importInfo.ePixelFormat == MTexturePixelFormat::Byte8)
	{
		stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, reqComp);
		pTextureData = LoadFromMemory("ImportFromMemoryTexture", (MByte*)data, unWidth, unHeight, reqComp, importInfo.ePixelFormat);
		stbi_image_free(data);
		data = nullptr;
	}
	else if (importInfo.ePixelFormat == MTexturePixelFormat::Float32)
	{
		float* data = stbi_loadf_from_memory((const stbi_uc*)buffer, nSize, &unWidth, &unHeight, &comp, reqComp);
		pTextureData = LoadFromMemory("ImportFromMemoryTexture", (MByte*)data, unWidth, unHeight, reqComp, importInfo.ePixelFormat);
		stbi_image_free(data);
		data = nullptr;
	}
	else
	{
		return pTextureData;
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

	std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	auto pResult = ImportTextureFromMemory(buffer.data(), buffer.size(), importInfo);
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
		if (importInfo.ePixelFormat == MTexturePixelFormat::Byte8)
		{
			stbi_uc* data = stbi_load_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &unChannel, 0);
			vImageData[fileIdx] = (MByte*)data;
		}
		else if (importInfo.ePixelFormat == MTexturePixelFormat::Float32)
		{
			float* data = stbi_loadf_from_memory((const stbi_uc*)buffer.data(), buffer.size(), &unWidth, &unHeight, &unChannel, 0);
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

	if (unCubeMapChannel == 1 || unCubeMapChannel == 4)
	{
		int nTextureSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;

		if (MTexturePixelFormat::Byte8 == importInfo.ePixelFormat)
		{
			nTextureSize *= sizeof(MByte);
			textureData->aByteData.resize(sizeof(MByte) * (nTextureSize * 6));
		}
		else if (MTexturePixelFormat::Float32 == importInfo.ePixelFormat)
		{
			nTextureSize += sizeof(float);
			textureData->aByteData.resize(sizeof(float) * (nTextureSize * 6));
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			memcpy(textureData->aByteData.data() + nTextureSize * nTexIdx, vImageData[nTexIdx], nTextureSize);
		}
	}
	else
	{
		int nSourceSize = unCubeMapWidth * unCubeMapHeight * unCubeMapChannel;
		int nTargetSize = unCubeMapWidth * unCubeMapHeight * 4;

		if (MTexturePixelFormat::Byte8 == importInfo.ePixelFormat)
		{
			textureData->aByteData.resize(sizeof(MByte) * (nTargetSize * 6 ));
		}
		else if (MTexturePixelFormat::Float32 == importInfo.ePixelFormat)
		{
			textureData->aByteData.resize(sizeof(float) * (nTargetSize * 6));
		}

		for (int nTexIdx = 0; nTexIdx < 6; ++nTexIdx)
		{
			if (MTexturePixelFormat::Byte8 == importInfo.ePixelFormat)
			{
				FillChannelNum<MByte>(vImageData[nTexIdx], (MByte*)(textureData->aByteData.data()) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0, 0, 0, 255 });
			}
			else if (MTexturePixelFormat::Float32 == importInfo.ePixelFormat)
			{
				FillChannelNum<float>(vImageData[nTexIdx], (float*)(textureData->aByteData.data()) + nTargetSize * nTexIdx, nSourceSize, unCubeMapChannel, 4, { 0.0f, 0.0f, 0.0f, 1.0f});
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
	textureData->nImageLayerNum = 6;
	textureData->nChannel = unCubeMapChannel;
	textureData->ePixelFormat = importInfo.ePixelFormat;
	textureData->strTextureName = vResourcePath[0];
	textureData->eTextureType = METextureType::ETextureCube;
	
#if	FREE_MEMORY_AFTER_UPLOAD
	std::swap(m_aByteData, std::vector<MByte>());
#endif

	return textureData;
}

METextureLayout MTextureResourceUtil::GetTextureLayout(const uint32_t& nChannel, const MTexturePixelFormat& format)
{
	METextureLayout eResult = METextureLayout::E_UNKNOW;
	if (MTexturePixelFormat::Byte8 == format)
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
	else if (MTexturePixelFormat::Float32 == format)
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
