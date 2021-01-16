#include "MRenderPass.h"
#include "MIRenderTarget.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanRenderTarget.h"
#endif

MSubpass::MSubpass()
	: m_strName("")
	, m_vInputIndex()
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

MPassTargetDescription::MPassTargetDescription(const bool bClear, const MColor& cColor)
	: m_strName("")
	, bClearWhenRender(bClear)
	, cClearColor(cColor)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkTargetFormat = VK_FORMAT_R8G8B8A8_SRGB;
#endif
}

MPassTargetDescription::MPassTargetDescription()
	: m_strName("")
	, bClearWhenRender(true)
	, cClearColor(MColor::Black)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_vkTargetFormat = VK_FORMAT_R8G8B8A8_SRGB;
#endif
}
