#include "MRenderPass.h"
#include "MIRenderTarget.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanRenderTarget.h"
#endif

MSubpass::MSubpass()
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

MRenderPass::MTargetDesc::MTargetDesc(const bool bClear, const MColor& cColor)
	: bClearWhenRender(bClear)
	, cClearColor(cColor)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkTargetFormat = VK_FORMAT_R8G8B8A8_SRGB;
#endif
}

MRenderPass::MTargetDesc::MTargetDesc()
	: bClearWhenRender(true)
	, cClearColor(MColor::Black)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkTargetFormat = VK_FORMAT_R8G8B8A8_SRGB;
#endif
}
