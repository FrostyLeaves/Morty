#include "MTexture.h"

#include "MIDevice.h"

MTexture::MTexture()
	: m_v2Size(1.0, 1.0)
	, m_eRenderType(METextureLayout::ERGBA8)
	, m_eRenderUsage(METextureRenderUsage::EUnknow)
	, m_eShaderUsage(METextureShaderUsage::EUnknow)
	, m_bReadable(false)
	, m_bMipmapsEnable(false)
	, m_unMipmapLevel(0)
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

void MTexture::GenerateBuffer(MIDevice* pDevice, MByte* aImageData /*= nullptr*/)
{
	pDevice->GenerateTexture(this, aImageData);
}

void MTexture::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyTexture(this);
}

uint32_t MTexture::GetImageMemorySize(const METextureLayout& layout)
{
	if (METextureLayout::ERGBA16 == layout)
		return 8;
	else
		return 4;
}

MTexture* MTexture::CreateShadowMap()
{
	MTexture* pTexture = new MTexture();
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderDepth);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::EDepth);

	return pTexture;
}
