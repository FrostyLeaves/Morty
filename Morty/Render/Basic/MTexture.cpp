#include "Basic/MTexture.h"
#include "Render/MIDevice.h"

using namespace morty;

MTexture::MTexture()
	: m_strTextureName("Texture_Default")
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

uint32_t MTexture::GetImageMemorySize(const METextureLayout& layout)
{
	switch (layout)
	{
	case METextureLayout::Unknow:

	case METextureLayout::UNorm_R8:
	case METextureLayout::UInt_R8:
		return 1;
	case METextureLayout::UNorm_RG8:
		return 2;
	case METextureLayout::UNorm_RGB8:
		return 3;
	case METextureLayout::UNorm_RGBA8:
		return 4;
	case METextureLayout::Float_R16:
		return 2;
	case METextureLayout::Float_RG16:
		return 4;
	case METextureLayout::Float_RGB16:
		return 6;
	case METextureLayout::Float_RGBA16:
		return 8;
	case METextureLayout::Float_R32:
		return 4;
	case METextureLayout::Float_RG32:
		return 8;
	case METextureLayout::Float_RGB32:
		return 12;
	case METextureLayout::Float_RGBA32:
		return 16;
	case METextureLayout::Depth:
		return 4;
	default:
		MORTY_ASSERT(false);
	}

	return 0;
}

std::shared_ptr<MTexture> MTexture::CreateTexture(const MTextureDesc& desc)
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName(desc.strTextureName);
	pTexture->SetSize(desc.n3Size);
	pTexture->SetLayer(desc.nLayer);
	pTexture->SetTextureType(desc.eTextureType);
	pTexture->SetTextureLayout(desc.eTextureLayout);
	pTexture->SetRenderUsage(desc.eWriteUsage);
	pTexture->SetShaderUsage(desc.nShaderUsage);
	pTexture->SetReadable(desc.bReadable);
	pTexture->SetMipmapDataType(desc.eMipmapDataType);

	return pTexture;
}

MTextureDesc MTexture::CreateDepthBuffer()
{
	MTextureDesc texture = {
		.strTextureName = "Depth Buffer Texture",
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::Depth,
		.eWriteUsage = METextureWriteUsage::ERenderDepth,
		.eMipmapDataType = MEMipmapDataType::Disable,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
	};

	return texture;
}

MTextureDesc MTexture::CreateShadowMapArray(const int& nSize, const uint32_t& nArraySize)
{
	MTextureDesc texture = {
		.strTextureName = "Shadow Map Texture Array",
		.n3Size = Vector3i(nSize, nSize, 1),
		.nLayer = nArraySize,
		.eTextureType = METextureType::ETexture2DArray,
		.eTextureLayout = METextureLayout::Depth,
		.eWriteUsage = METextureWriteUsage::ERenderDepth,
		.eMipmapDataType = MEMipmapDataType::Disable,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTarget(METextureLayout eLayout/*= METextureLayout::UNorm_RGBA8*/)
{
	MTextureDesc texture = {
		.strTextureName = "Render Target Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = eLayout,
		.eWriteUsage = METextureWriteUsage::ERenderBack,
		.eMipmapDataType = MEMipmapDataType::Disable,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTargetGBuffer()
{
	MTextureDesc texture = {
		.strTextureName = "GBuffer Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::Float_RGBA16,
		.eWriteUsage = METextureWriteUsage::ERenderBack,
		.eMipmapDataType = MEMipmapDataType::Disable,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTargetFloat32()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Render Target Float32 Texture");
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureWriteUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler);
	pTexture->SetTextureLayout(METextureLayout::Float_R32);
	pTexture->SetMipmapDataType(MEMipmapDataType::Disable);

	return pTexture;
}

MTextureDesc MTexture::CreateShadingRate()
{
	MTextureDesc texture = {
		.strTextureName = "Shading Rate Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::UInt_R8,
		.eWriteUsage = METextureWriteUsage::EStorageWrite,
		.eMipmapDataType = MEMipmapDataType::Disable,
#if MORTY_DEBUG
		.nShaderUsage = (METextureReadUsage::EShadingRateMask | METextureReadUsage::EPixelSampler),
#else
		.nShaderUsage = METextureReadUsage::EShadingRateMask,
#endif
		.bReadable = false,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateVXGIMap()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("VXGI Texture");
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureWriteUsage::EStorageWrite);
	pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler | METextureReadUsage::EStorageRead);
	pTexture->SetTextureLayout(METextureLayout::Float_RGBA32);
	pTexture->SetTextureType(METextureType::ETexture3D);
	pTexture->SetMipmapDataType(MEMipmapDataType::Disable);

	return pTexture;
}
