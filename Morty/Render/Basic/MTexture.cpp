#include "Basic/MTexture.h"

#include "Render/MIDevice.h"

MTexture::MTexture()
	: m_strTextureName("Texture_Default")
	, m_n3Size(1, 1, 1)
	, m_eRenderType(METextureLayout::ERGBA_UNORM_8)
	, m_eRenderUsage(METextureWriteUsage::EUnknow)
	, m_eShaderUsage(METextureReadUsage::EUnknow)
	, m_eTextureType(METextureType::ETexture2D)
	, m_bReadable(false)
	, m_bMipmapsEnable(false)
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

void MTexture::GenerateBuffer(MIDevice* pDevice, const MByte* aImageData /*= nullptr*/)
{
	pDevice->GenerateTexture(this, aImageData);
}

void MTexture::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyTexture(this);
}

uint32_t MTexture::GetImageMemorySize(const METextureLayout& layout)
{
	switch (layout)
	{
	case METextureLayout::E_UNKNOW:

	case METextureLayout::ER_UNORM_8:
	case METextureLayout::ER_UINT_8:
		return 1;
	case METextureLayout::ERG_UNORM_8:
		return 2;
	case METextureLayout::ERGB_UNORM_8:
		return 3;
	case METextureLayout::ERGBA_UNORM_8:
		return 4;
	case METextureLayout::ER_FLOAT_16:
		return 2;
	case METextureLayout::ERG_FLOAT_16:
		return 4;
	case METextureLayout::ERGB_FLOAT_16:
		return 6;
	case METextureLayout::ERGBA_FLOAT_16:
		return 8;
	case METextureLayout::ER_FLOAT_32:
		return 4;
	case METextureLayout::ERG_FLOAT_32:
		return 8;
	case METextureLayout::ERGB_FLOAT_32:
		return 12;
	case METextureLayout::ERGBA_FLOAT_32:
		return 16;
	case METextureLayout::EDepth:
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
	pTexture->SetMipmapsEnable(desc.bMipmapEnable);

	return pTexture;
}

MTextureDesc MTexture::CreateDepthBuffer()
{
	MTextureDesc texture = {
		.strTextureName = "Depth Buffer Texture",
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::EDepth,
		.eWriteUsage = METextureWriteUsage::ERenderDepth,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
		.bMipmapEnable = false,
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
		.eTextureLayout = METextureLayout::EDepth,
		.eWriteUsage = METextureWriteUsage::ERenderDepth,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
	    .bMipmapEnable = false,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTarget(METextureLayout eLayout/*= METextureLayout::ERGBA_UNORM_8*/)
{
	MTextureDesc texture = {
		.strTextureName = "Render Target Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = eLayout,
		.eWriteUsage = METextureWriteUsage::ERenderBack,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
		.bMipmapEnable = false,
	};

	return texture;
}

MTextureDesc MTexture::CreateRenderTargetGBuffer()
{
	MTextureDesc texture = {
		.strTextureName = "GBuffer Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::ERGBA_FLOAT_16,
		.eWriteUsage = METextureWriteUsage::ERenderBack,
		.nShaderUsage = METextureReadUsage::EPixelSampler,
		.bReadable = false,
		.bMipmapEnable = false,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTargetFloat32()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Render Target Float32 Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureWriteUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler);
	pTexture->SetTextureLayout(METextureLayout::ER_FLOAT_32);

	return pTexture;
}

MTextureDesc MTexture::CreateShadingRate()
{
	MTextureDesc texture = {
		.strTextureName = "Shading Rate Texture",
		.n3Size = Vector3i(1, 1, 1),
		.eTextureType = METextureType::ETexture2D,
		.eTextureLayout = METextureLayout::ER_UINT_8,
		.eWriteUsage = METextureWriteUsage::EStorageWrite,
#if MORTY_DEBUG
		.nShaderUsage = (METextureReadUsage::EShadingRateMask | METextureReadUsage::EPixelSampler),
#else
		.nShaderUsage = METextureReadUsage::EShadingRateMask,
#endif
		.bReadable = false,
		.bMipmapEnable = false,
	};

	return texture;
}

std::shared_ptr<MTexture> MTexture::CreateVXGIMap()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("VXGI Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureWriteUsage::EStorageWrite);
	pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler | METextureReadUsage::EStorageRead);
	pTexture->SetTextureLayout(METextureLayout::ERGBA_FLOAT_32);
	pTexture->SetTextureType(METextureType::ETexture3D);

	return pTexture;
}
