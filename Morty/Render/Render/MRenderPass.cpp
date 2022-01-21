#include "MRenderPass.h"

#include "MIDevice.h"


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
	, m_vBackDesc()
	, m_DepthDesc()
	, m_unRenderPassID(MGlobal::M_INVALID_INDEX)
	, m_vBackTextures()
	, m_pDepthTexture(nullptr)
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

std::vector<MTexture*> MRenderPass::GetBackTexture()
{
	return m_vBackTextures;
}

MTexture* MRenderPass::GetDepthTexture()
{
	return m_pDepthTexture;
}

MPassTargetDescription::MPassTargetDescription(const bool bClear, const MColor& cColor)
	: bClearWhenRender(bClear)
	, bAlreadyRender(false)
	, cClearColor(cColor)
{
}

MPassTargetDescription::MPassTargetDescription()
	: bClearWhenRender(true)
	, bAlreadyRender(false)
	, cClearColor(MColor::Black)
{
}

MPassTargetDescription::MPassTargetDescription(const bool bClear, const bool bAlready, const MColor& cClearColor)
	: bClearWhenRender(bClear)
	, bAlreadyRender(bAlready)
	, cClearColor(cClearColor)
{

}
