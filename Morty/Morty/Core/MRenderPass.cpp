#include "MRenderPass.h"
#include "MIRenderTarget.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanRenderTarget.h"
#endif

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
	, m_unRenderPassID(0)
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_aVkRenderPass.fill(VK_NULL_HANDLE);
#endif
}

MRenderPass::~MRenderPass()
{

}

void MRenderPass::GenerateBuffer(MIDevice* pDevice)
{

}

void MRenderPass::DestroyBuffer(MIDevice* pDevice)
{

}

std::vector<MIRenderTexture*> MRenderPass::GetBackTexture(const size_t& nFrameIdx)
{
	return m_aFrameBuffers[nFrameIdx].vBackTextures;
}

MIRenderTexture* MRenderPass::GetDepthTexture(const size_t& nFrameIdx)
{
	return m_aFrameBuffers[nFrameIdx].pDepthTexture;
}

MFrameBuffer* MRenderPass::GetFrameBuffer(const size_t& nFrameIdx)
{
	return &m_aFrameBuffers[nFrameIdx];
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
