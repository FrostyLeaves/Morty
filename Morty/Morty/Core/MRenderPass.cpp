#include "MRenderPass.h"

MSubpass::MSubpass()
{

}

MSubpass::~MSubpass()
{
}

MRenderPass::MRenderPass(MIRenderTarget* pRenderTarget)
	: m_pRenderTarget(pRenderTarget)
	, m_vSubpass()
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
