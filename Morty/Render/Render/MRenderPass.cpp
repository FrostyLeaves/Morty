#include "Render/MRenderPass.h"

#include "Render/MIDevice.h"


MSubpass::MSubpass()
	: m_vInputIndex()
	, m_vOutputIndex()
	, m_unViewMask(0b11111111)
	, m_unCorrelationMask(0b11111111)
{

}

MSubpass::~MSubpass()
{
}

MSubpass::MSubpass(const std::vector<uint32_t>& inputs, const std::vector<uint32_t>& outputs)
	: m_vInputIndex(inputs)
	, m_vOutputIndex(outputs)
	, m_unViewMask(0b11111111)
	, m_unCorrelationMask(0b11111111)
{
}

MRenderPass::MRenderPass()
	: m_vSubpass()
	, m_unRenderPassID(MGlobal::M_INVALID_INDEX)
	, m_vBackTextures()
	, m_DepthTexture()
	, m_unViewNum(1)
{

#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkExtent2D = VkExtent2D();
	m_VkFrameBuffer = VK_NULL_HANDLE;
	m_VkRenderPass = VK_NULL_HANDLE;
#endif
}

MRenderPass::~MRenderPass()
{

}

void MRenderPass::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->GenerateRenderPass(this);
	pDevice->GenerateFrameBuffer(this);
}

void MRenderPass::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyFrameBuffer(this);
	pDevice->DestroyRenderPass(this);
}

void MRenderPass::Resize(MIDevice* pDevice)
{
	pDevice->DestroyFrameBuffer(this);
	pDevice->GenerateFrameBuffer(this);
}

Vector2 MRenderPass::GetFrameBufferSize()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	return Vector2(m_vkExtent2D.width, m_vkExtent2D.height);
#endif

	return Vector2();
}

void MRenderPass::AddBackTexture(std::shared_ptr<MTexture> pBackTexture, const MPassTargetDescription& desc)
{
	MBackTexture backTexture;
	backTexture.pTexture = pBackTexture;
	backTexture.desc = desc;

	m_vBackTextures.push_back(backTexture);
}

void MRenderPass::SetDepthTexture(std::shared_ptr<MTexture> pDepthTexture, const MPassTargetDescription& desc)
{
	m_DepthTexture.pTexture = pDepthTexture;
	m_DepthTexture.desc = desc;
}

std::vector<std::shared_ptr<MTexture>> MRenderPass::GetBackTextures()
{
	std::vector<std::shared_ptr<MTexture>> vTextures;

	for (MBackTexture& tex : m_vBackTextures)
		vTextures.push_back(tex.pTexture);

	return vTextures;
}

std::shared_ptr<MTexture> MRenderPass::GetDepthTexture()
{
	return m_DepthTexture.pTexture;
}

MPassTargetDescription::MPassTargetDescription(const bool bClear, const MColor& cColor)
	: bClearWhenRender(bClear)
	, bAlreadyRender(false)
	, cClearColor(cColor)
	, nMipmapLevel(0)
{
}

MPassTargetDescription::MPassTargetDescription()
	: bClearWhenRender(true)
	, bAlreadyRender(false)
	, cClearColor(MColor::Black)
	, nMipmapLevel(0)
{
}

MPassTargetDescription::MPassTargetDescription(const bool bClear, const bool bAlready, const MColor& cColor)
	: bClearWhenRender(bClear)
	, bAlreadyRender(bAlready)
	, cClearColor(cColor)
	, nMipmapLevel(0)
{

}

MPassTargetDescription::MPassTargetDescription(const bool bClear, const bool bAlready, const MColor& cColor, const uint32_t& nMipmap)
	: bClearWhenRender(bClear)
	, bAlreadyRender(bAlready)
	, cClearColor(cColor)
	, nMipmapLevel(nMipmap)
{

}

MBackTexture::MBackTexture()
	: pTexture(nullptr)
	, desc()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkImageView = VK_NULL_HANDLE;
#endif
}
