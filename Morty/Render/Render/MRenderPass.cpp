#include "MRenderPass.h"

#include "MIDevice.h"


MSubpass::MSubpass()
	: m_vInputIndex()
	, m_vOutputIndex()
{

}

MSubpass::~MSubpass()
{
}

MRenderPass::MRenderPass()
	: m_vSubpass()
	, m_vBackDesc()
	, m_DepthDesc()
	, m_unRenderPassID(MGlobal::M_INVALID_INDEX)
	, m_vBackTextures()
	, m_pDepthTexture(nullptr)
{

#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkExtent2D = VkExtent2D();
	m_VkFrameBuffer = VK_NULL_HANDLE;
	m_VkRenderPass = VK_NULL_HANDLE;
	m_VkCommandBuffers = VK_NULL_HANDLE;
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
	, cClearColor(cColor)
{
}

MPassTargetDescription::MPassTargetDescription()
	: bClearWhenRender(true)
	, cClearColor(MColor::Black)
{
}
