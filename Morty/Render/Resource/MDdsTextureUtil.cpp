#include "Resource/MDdsTextureUtil.h"

#include "Engine/MEngine.h"
#include "MTextureResource.h"
#include "RHI/Abstract/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

#define FOUR_CHAR_CODE(char1, char2, char3, char4)                                                                     \
    (static_cast<uint32_t>(char1) | (static_cast<uint32_t>(char2) << 8) | (static_cast<uint32_t>(char3) << 16) |       \
     (static_cast<uint32_t>(char4) << 24))

struct DdsPixelFormatType {
    static constexpr uint32_t ALPHA     = 0x2;
    static constexpr uint32_t FOURCC    = 0x4;
    static constexpr uint32_t RGB       = 0x40;
    static constexpr uint32_t RGBA      = 0x41;
    static constexpr uint32_t YUV       = 0x200;
    static constexpr uint32_t LUMINANCE = 0x20000;
};

struct DdsFilePixelFormat {
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t bitCount;
    uint32_t rBitMask;
    uint32_t gBitMask;
    uint32_t bBitMask;
    uint32_t aBitMask;
};

struct DdsHeader {
    uint32_t           magic;
    uint32_t           size;
    uint32_t           flags;
    uint32_t           height;
    uint32_t           width;
    uint32_t           pitch;
    uint32_t           depth;
    uint32_t           mipmapCount;
    uint32_t           reserved[11];
    DdsFilePixelFormat pixelFormat;
    uint32_t           caps1;
    uint32_t           caps2;
    uint32_t           caps3;
    uint32_t           caps4;
    uint32_t           reserved2;
};

struct Dx10Header {
    uint32_t dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlags;
    uint32_t arraySize;
    uint32_t miscFlags2;
};

struct DdsMagicNumber {
    static constexpr uint32_t Dds  = FOUR_CHAR_CODE('D', 'D', 'S', ' ');
    static constexpr uint32_t DXT1 = FOUR_CHAR_CODE('D', 'X', 'T', '1');
    static constexpr uint32_t DXT2 = FOUR_CHAR_CODE('D', 'X', 'T', '2');
    static constexpr uint32_t DXT3 = FOUR_CHAR_CODE('D', 'X', 'T', '3');
    static constexpr uint32_t DXT4 = FOUR_CHAR_CODE('D', 'X', 'T', '4');
    static constexpr uint32_t DXT5 = FOUR_CHAR_CODE('D', 'X', 'T', '5');
    static constexpr uint32_t ATI1 = FOUR_CHAR_CODE('A', 'T', 'I', '1');
    static constexpr uint32_t BC4U = FOUR_CHAR_CODE('B', 'C', '4', 'U');
    static constexpr uint32_t BC4S = FOUR_CHAR_CODE('B', 'C', '4', 'S');
    static constexpr uint32_t ATI2 = FOUR_CHAR_CODE('A', 'T', 'I', '2');
    static constexpr uint32_t BC5U = FOUR_CHAR_CODE('B', 'C', '5', 'U');
    static constexpr uint32_t BC5S = FOUR_CHAR_CODE('B', 'C', '5', 'S');
    static constexpr uint32_t RGBG = FOUR_CHAR_CODE('R', 'G', 'B', 'G');
    static constexpr uint32_t GRBG = FOUR_CHAR_CODE('G', 'R', 'B', 'G');
    static constexpr uint32_t YUY2 = FOUR_CHAR_CODE('Y', 'U', 'Y', '2');
    static constexpr uint32_t UYVY = FOUR_CHAR_CODE('U', 'Y', 'V', 'Y');
    static constexpr uint32_t DX10 = FOUR_CHAR_CODE('D', 'X', '1', '0');
};

constexpr uint32_t Dds_MAGIC_CONSTANT = 0x20534444;

METextureFormat    GetFormatFromDxFormat(uint32_t dxFormat)
{
    static const std::unordered_map<uint32_t, METextureFormat> FormatTable = {
            {0, METextureFormat::Unknow},
            {28, METextureFormat::UNorm_RGBA8},
            {71, METextureFormat::UNorm_RGBA8_BC1},
            {74, METextureFormat::UNorm_RGBA8_BC2},
            {77, METextureFormat::UNorm_RGBA8_BC3},
            {80, METextureFormat::UNorm_RGBA8_BC4},
            {81, METextureFormat::SNorm_RGBA8_BC4},
            {83, METextureFormat::UNorm_RGBA8_BC5},
            {84, METextureFormat::SNorm_RGBA8_BC5},
            {98, METextureFormat::UNorm_RGBA8_BC7},
    };

    const auto findResult = FormatTable.find(dxFormat);
    if (findResult == FormatTable.end())
    {
        MORTY_ASSERT(false);
        return METextureFormat::Unknow;
    }

    return findResult->second;
}

METextureFormat GetFormatFromFourChar(const uint32_t& fourCC)
{
    if (fourCC)
    {
        switch (fourCC)
        {
                // clang-format off
		case DdsMagicNumber::DXT1:
			return METextureFormat::UNorm_RGBA8_BC1;
		case DdsMagicNumber::DXT2:
		case DdsMagicNumber::DXT3:
			return METextureFormat::UNorm_RGBA8_BC2;
		case DdsMagicNumber::DXT4:
		case DdsMagicNumber::DXT5:
			return METextureFormat::UNorm_RGBA8_BC3;
		case DdsMagicNumber::ATI1:
		case DdsMagicNumber::BC4U:
			return METextureFormat::UNorm_RGBA8_BC4;
		case DdsMagicNumber::BC4S:
			return METextureFormat::SNorm_RGBA8_BC4;
		case DdsMagicNumber::ATI2:
		case DdsMagicNumber::BC5U:
			return METextureFormat::UNorm_RGBA8_BC5;
		case DdsMagicNumber::BC5S:
			return METextureFormat::SNorm_RGBA8_BC5;
		default:
			return METextureFormat::Unknow;
		}
	}

	return METextureFormat::Unknow;
}

uint32_t getBlockSize(METextureFormat layout)
{
	switch (layout) {
	case METextureFormat::UNorm_RGBA8_BC1:
	case METextureFormat::UNorm_RGBA8_BC4:
	case METextureFormat::SNorm_RGBA8_BC4:
		return 8;
	case METextureFormat::UNorm_RGBA8_BC2:
	case METextureFormat::UNorm_RGBA8_BC3:
	case METextureFormat::UNorm_RGBA8_BC5:
	case METextureFormat::SNorm_RGBA8_BC5:
		return 16;
	default:
		MORTY_ASSERT(false);
		return 0;
	}
}

uint32_t ComputeMipmapSize(METextureFormat layout, uint32_t width, uint32_t height)
{
    if (layout == METextureFormat::UNorm_RGBA8)
    {
		return ((width + 1) >> 1) * 4 * height;
    }

	const uint32_t pitch = std::max(1u, (width + 3) / 4) * getBlockSize(layout);
	return pitch * std::max(1u, (height + 3) / 4);
}

std::unique_ptr<MResourceData> MDdsTextureUtil::ImportDdsTexture(const MString& strResourcePath)
{
	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
		return nullptr;
	}

	size_t offset = 0;

	std::vector<MByte> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	const DdsHeader& header = *reinterpret_cast<DdsHeader*>(buffer.data());
	offset += sizeof(DdsHeader);

	MORTY_ASSERT(header.magic == Dds_MAGIC_CONSTANT);

	METextureFormat eTextureLayout = METextureFormat::Unknow;

	if ((header.pixelFormat.flags & DdsPixelFormatType::FOURCC) == DdsPixelFormatType::FOURCC)
	{
		if ((header.pixelFormat.fourCC & DdsMagicNumber::DX10) == DdsMagicNumber::DX10)
		{
			const Dx10Header& dxHeader = *reinterpret_cast<Dx10Header*>(buffer.data() + offset);
			offset += sizeof(Dx10Header);

			if (buffer.size() < 148)
			{
				return nullptr;
			}

			//TODO: not support texture array.
			MORTY_ASSERT(1 == dxHeader.arraySize);
			//TODO: only support Texture2D.
			MORTY_ASSERT(3 == dxHeader.resourceDimension);

			eTextureLayout = GetFormatFromDxFormat(dxHeader.dxgiFormat);
		}
        else
        {
			eTextureLayout = GetFormatFromFourChar(header.pixelFormat.fourCC);
        }

	}
	else if ((header.pixelFormat.flags & DdsPixelFormatType::RGBA) == DdsPixelFormatType::RGBA)
	{
		eTextureLayout = METextureFormat::UNorm_RGBA8;
	}
    else
    {
		MORTY_ASSERT(false);
    }
	
	std::unique_ptr<MTextureResourceData> textureData = std::make_unique<MTextureResourceData>();

	textureData->nWidth = header.width;
	textureData->nHeight = header.height;
	textureData->nDepth = 1;
	textureData->eFormat = static_cast<morty::METextureFormat>(eTextureLayout);
	textureData->strTextureName = strResourcePath;
	textureData->eMipmapDataType = MEMipmapDataType::Load;

	//mipmaps
	const auto nMipmapCount = std::max(header.mipmapCount, 1u);
	uint32_t nWidth = header.width;
	uint32_t nHeight = header.height;

	uint32_t nMipmapOffset = offset;
	for (uint32_t mipIdx = 0; mipIdx < nMipmapCount; ++mipIdx)
	{
	    uint32_t mipmapDataSize = ComputeMipmapSize(eTextureLayout, nWidth, nHeight);
		MORTY_ASSERT(mipmapDataSize);

		mipmapDataSize = std::min(mipmapDataSize, static_cast<uint32_t>(buffer.size() - nMipmapOffset));

		textureData->vMipmaps.push_back({ buffer.begin() + nMipmapOffset, buffer.begin() + nMipmapOffset + mipmapDataSize });

		nMipmapOffset += mipmapDataSize;
		nWidth = std::max(nWidth / 2, 1u);
		nHeight = std::max(nHeight / 2, 1u);
	}

	return textureData;
}
