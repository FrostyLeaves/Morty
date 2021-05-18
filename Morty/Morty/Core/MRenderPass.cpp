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
	, m_unRenderPassID(MGlobal::M_INVALID_INDEX)
	, m_VkRenderPass(VK_NULL_HANDLE)
	, m_FrameBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
}

MRenderPass::~MRenderPass()
{

}

void MRenderPass::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->DestroyRenderPass(this);
	pDevice->DestroyFrameBuffer(this);
}

void MRenderPass::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyFrameBuffer(this);
	pDevice->DestroyRenderPass(this);
}

std::vector<MIRenderTexture*> MRenderPass::GetBackTexture()
{
	return m_FrameBuffer.vBackTextures;
}

MIRenderTexture* MRenderPass::GetDepthTexture()
{
	return m_FrameBuffer.pDepthTexture;
}

MFrameBuffer* MRenderPass::GetFrameBuffer()
{
	return &m_FrameBuffer;
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
