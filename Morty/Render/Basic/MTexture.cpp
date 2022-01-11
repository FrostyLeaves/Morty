﻿#include "MTexture.h"

#include "MIDevice.h"

MTexture::MTexture()
	: m_strTextureName()
	, m_v2Size(1.0, 1.0)
	, m_eRenderType(METextureLayout::ERGBA_UNORM_8)
	, m_eRenderUsage(METextureRenderUsage::EUnknow)
	, m_eShaderUsage(METextureShaderUsage::EUnknow)
	, m_eTextureType(METextureType::ETexture2D)
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
	if (METextureLayout::ERGBA_FLOAT_16 == layout)
		return 8;
	else if (METextureLayout::ERGBA_FLOAT_32 == layout)
		return 16;
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

MTexture* MTexture::CreateRenderTarget()
{
	MTexture* pTexture = new MTexture();
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);

	return pTexture;
}

MTexture* MTexture::CreateRenderTargetFloat32()
{
	MTexture* pTexture = new MTexture();
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::ERenderBack);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ER_FLOAT_32);

	return pTexture;
}

MTexture* MTexture::CreateCubeMap()
{
	MTexture* pTexture = new MTexture();
	pTexture->SetMipmapsEnable(false);
	pTexture->SetReadable(false);
	pTexture->SetRenderUsage(METextureRenderUsage::EUnknow);
	pTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	pTexture->SetTextureType(METextureType::ETextureCube);

	return pTexture;
}
