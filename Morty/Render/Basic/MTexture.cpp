#include "Basic/MTexture.h"
#include "Render/MIDevice.h"

using namespace morty;

MTexture::MTexture()
{

#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkTextureFormat = VK_FORMAT_R8G8B8A8_SRGB;
	m_VkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_VkTextureImage = VK_NULL_HANDLE;
	m_VkTextureImageMemory = VK_NULL_HANDLE;
	m_VkImageView = VK_NULL_HANDLE;
	m_VkSampler = VK_NULL_HANDLE;
#endif
}

MTexture::~MTexture()
{

}

Vector2 MTexture::GetMipmapSize(const uint32_t& nMipmapLevel)
{
	uint32_t w = static_cast<uint32_t>(GetSize().x);
	uint32_t h = static_cast<uint32_t>(GetSize().y);

	for (uint32_t i = 0; i < nMipmapLevel; ++i)
	{
		w = w / 2;
		h = h / 2;
	}

	return Vector2(w, h);
}

void MTexture::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->GenerateTexture(this, {});
}

void MTexture::GenerateBuffer(MIDevice* pDevice, const std::vector<std::vector<MByte>>& buffer)
{
	pDevice->GenerateTexture(this, buffer);
}

void MTexture::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyTexture(this);
}

uint32_t MTexture::GetImageMemorySize(const METextureFormat& layout)
{
	switch (layout)
	{
	case METextureFormat::Unknow:

	case METextureFormat::UNorm_R8:
	case METextureFormat::UInt_R8:
		return 1;
	case METextureFormat::UNorm_RG8:
		return 2;
	case METextureFormat::UNorm_RGB8:
		return 3;
	case METextureFormat::UNorm_RGBA8:
		return 4;
	case METextureFormat::Float_R16:
		return 2;
	case METextureFormat::Float_RG16:
		return 4;
	case METextureFormat::Float_RGB16:
		return 6;
	case METextureFormat::Float_RGBA16:
		return 8;
	case METextureFormat::Float_R32:
		return 4;
	case METextureFormat::Float_RG32:
		return 8;
	case METextureFormat::Float_RGB32:
		return 12;
	case METextureFormat::Float_RGBA32:
		return 16;
	case METextureFormat::Depth:
		return 4;
	default:
		MORTY_ASSERT(false);
	}

	return 0;
}

std::shared_ptr<MTexture> MTexture::CreateTexture(const MTextureDesc& desc)
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
    pTexture->m_desc = desc;
    pTexture->m_desc.nLayer = desc.eTextureType == METextureType::ETextureCube ? 6 : desc.nLayer;

	return pTexture;
}

MTextureDesc MTexture::CreateDepthBuffer()
{
	MTextureDesc texture = {
		.strName = "Depth Buffer Texture",
		.eTextureType = METextureType::ETexture2D,
		.eFormat = METextureFormat::Depth,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler,
		.nWriteUsage = METextureWriteUsageBit::ERenderDepth,
	};

	return texture;
}

MTextureDesc MTexture::CreateShadowMapArray(const int& nSize, const uint32_t& nArraySize)
{
	MTextureDesc texture = {
		.strName = "Shadow Map Texture Array",
		.n3Size = Vector3i(nSize, nSize, 1),
		.nLayer = nArraySize,
		.eTextureType = METextureType::ETexture2DArray,
		.eFormat = METextureFormat::Depth,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler,
		.nWriteUsage = METextureWriteUsageBit::ERenderDepth,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTarget(METextureFormat eFormat/*= METextureFormat::UNorm_RGBA8*/)
{
	MTextureDesc texture = {
		.strName = "Render Target Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eFormat = eFormat,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler,
		.nWriteUsage = METextureWriteUsageBit::ERenderBack,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTargetGBuffer()
{
	MTextureDesc texture = {
		.strName = "GBuffer Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eFormat = METextureFormat::Float_RGBA16,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler,
		.nWriteUsage = METextureWriteUsageBit::ERenderBack,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTargetFloat32()
{
	std::shared_ptr<MTexture> pTexture = MTexture::CreateTexture({
        .strName = "Render Target Float32 Texture",
        .eFormat = METextureFormat::Float_R32,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler,
        .nWriteUsage = METextureWriteUsageBit::ERenderBack,
    });

	return pTexture;
}

MTextureDesc MTexture::CreateShadingRate()
{
	MTextureDesc texture = {
		.strName = "Shading Rate Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eFormat = METextureFormat::UInt_R8,
        .eMipmapDataType = MEMipmapDataType::Disable,
#if MORTY_DEBUG
		.nReadUsage = (METextureReadUsageBit::EShadingRateMask | METextureReadUsageBit::EPixelSampler),
#else
		.nReadUsage = METextureReadUsageBit::EShadingRateMask,
#endif
        .nWriteUsage = METextureWriteUsageBit::EStorageWrite,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateVXGIMap(Vector3i n3Size = Vector3i::One)
{
    std::shared_ptr<MTexture> pTexture = MTexture::CreateTexture({
        .strName = "VXGI Texture",
        .n3Size = n3Size,
        .eTextureType = METextureType::ETexture3D,
        .eFormat = METextureFormat::Float_RGBA32,
        .eMipmapDataType = MEMipmapDataType::Disable,
        .nReadUsage = METextureReadUsageBit::EPixelSampler | METextureReadUsageBit::EStorageRead,
        .nWriteUsage = METextureWriteUsageBit::EStorageWrite,
    });

	return pTexture;
}

void MTexture::Resize(MIDevice *pDevice, const Vector2i &n2Size) {
    DestroyBuffer(pDevice);
    m_desc.n3Size.x = n2Size.x;
    m_desc.n3Size.y = n2Size.y;
    GenerateBuffer(pDevice);
}

void MTexture::Resize(MIDevice *pDevice, const Vector3i &n3Size) {
    DestroyBuffer(pDevice);
    m_desc.n3Size = n3Size;
    GenerateBuffer(pDevice);
}
