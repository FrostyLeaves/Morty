#include "Resource/MAstcTextureUtil.h"

#include "MTextureResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

struct AstcHeader
{
	uint8_t magic[4];
	uint8_t blockdim_x;
	uint8_t blockdim_y;
	uint8_t blockdim_z;
	uint8_t xsize[3];        // x-size = xsize[0] + xsize[1] + xsize[2]
	uint8_t ysize[3];        // x-size, y-size and z-size are given in texels;
	uint8_t zsize[3];        // block count is inferred
};

constexpr uint32_t ASTC_MAGIC_CONSTANT = 0x5CA1AB13;

std::unique_ptr<MResourceData> MAstcTextureUtil::ImportAstcTexture(const MString& strResourcePath)
{
	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
		return nullptr;
	}

	std::vector<MByte> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	AstcHeader header{};
	std::memcpy(&header, buffer.data(), sizeof(AstcHeader));
	const uint32_t nMagicValue = header.magic[0] + 256 * static_cast<uint32_t>(header.magic[1]) + 65536 * static_cast<uint32_t>(header.magic[2]) + 16777216 * static_cast<uint32_t>(header.magic[3]);
	if (nMagicValue != ASTC_MAGIC_CONSTANT)
	{
		MORTY_ASSERT(false);
		return nullptr;
	}

	const Vector3i blockdim = {
		/* xdim = */ static_cast<int>(header.blockdim_x),
		/* ydim = */ static_cast<int>(header.blockdim_y),
		/* zdim = */ static_cast<int>(header.blockdim_z)
	};

	const Vector3i extent = {
		/* width  = */ static_cast<int>(header.xsize[0] + 256 * header.xsize[1] + 65536 * header.xsize[2]),
		/* height = */ static_cast<int>(header.ysize[0] + 256 * header.ysize[1] + 65536 * header.ysize[2]),
		/* depth  = */ static_cast<int>(header.zsize[0] + 256 * header.zsize[1] + 65536 * header.zsize[2]) };


	const auto eFormat = MAstcTextureUtil::GetTextureFormat(blockdim);

	std::unique_ptr<MTextureResourceData> textureData = std::make_unique<MTextureResourceData>();

	textureData->aByteData = std::vector<MByte>{ buffer.begin() + sizeof(AstcHeader), buffer.end() };
	textureData->nWidth = extent.x;
	textureData->nHeight = extent.y;
	textureData->nDepth = extent.z;
	textureData->eFormat = eFormat;
	textureData->strTextureName = strResourcePath;
	textureData->bMipmap = false;

	return textureData;
}

morty::METextureLayout MAstcTextureUtil::GetTextureFormat(const Vector3i& n3BlockDim)
{
	MORTY_ASSERT(n3BlockDim.x == n3BlockDim.y && n3BlockDim.x == n3BlockDim.z);


	if (n3BlockDim.x == 4)
	{
		return morty::METextureLayout::UNorm_RGBA8_ASTC4x4;
	}

    if (n3BlockDim.x == 8)
	{
		return morty::METextureLayout::UNorm_RGBA8_ASTC8x8;
	}

	MORTY_ASSERT(false);
	return morty::METextureLayout::Unknow;
}
