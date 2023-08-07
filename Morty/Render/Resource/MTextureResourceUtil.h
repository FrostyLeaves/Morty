/**
 * @File         MTextureResourceUtil
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MTextureResource.h"

struct MTextureResourceData;

class MORTY_API MTextureResourceUtil
{
public:

	static std::unique_ptr<MResourceData> ImportTexture(const MString&  strResourcePath, const MTextureImportInfo& importInfo);
	static std::unique_ptr<MResourceData> ImportTextureFromMemory(char* buffer, size_t nSize, const MTextureImportInfo& importInfo);

	static std::unique_ptr<MResourceData> ImportCubeMap(const std::array<MString, 6>& vResourcePath, const MTextureImportInfo& importInfo);
	
	static std::unique_ptr<MResourceData> LoadFromMemory(const MString& strTextureName, const MByte* aByteData, const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, MTexturePixelFormat ePixelFormat = MTexturePixelFormat::Byte8);


	static METextureLayout GetTextureLayout(const uint32_t& nChannel, const MTexturePixelFormat& format);
};
