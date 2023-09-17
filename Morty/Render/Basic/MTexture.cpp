#include "Basic/MTexture.h"

#include "Render/MIDevice.h"

MTexture::MTexture()
	: m_strTextureName("Texture_Default")
	, m_v2Size(1.0, 1.0)
	, m_eRenderType(METextureLayout::ERGBA_UNORM_8)
	, m_eRenderUsage(METextureRenderUsage::EUnknow)
	, m_eShaderUsage(METextureShaderUsage::EUnknow)
	, m_eTextureType(METextureType::ETexture2D)
	, m_bReadable(false)
	, m_bMipmapsEnable(false)
	, m_unImageLayerNum(1)
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

std::shared_ptr<MTexture> MTexture::CreateShadowMap()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Shadow Map Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderDepth);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::EDepth);

	return pTexture;
}

std::shared_ptr<MTexture> MTexture::CreateShadowMapArray(const size_t& nArraySize)
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Shadow Map Texture Array");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderDepth);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::EDepth);
	pTexture->SetTextureType(METextureType::ETexture2DArray);
	pTexture->SetImageLayerNum(nArraySize);

	return pTexture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTarget(METextureLayout eLayout/*= METextureLayout::ERGBA_UNORM_8*/)
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Render Target Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(eLayout);

	return pTexture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTargetGBuffer()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("GBuffer Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ERGBA_FLOAT_16);

	return pTexture;
}

std::shared_ptr<MTexture> MTexture::CreateRenderTargetFloat32()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("Render Target Float32 Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ER_FLOAT_32);

	return pTexture;
}

std::shared_ptr<MTexture> MTexture::CreateCubeMap()
{
	std::shared_ptr<MTexture> pTexture = std::make_shared<MTexture>();
	pTexture->SetName("CubeMap Texture");
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::EUnknow);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	pTexture->SetTextureType(METextureType::ETextureCube);
	pTexture->SetImageLayerNum(6);

	return pTexture;
}
