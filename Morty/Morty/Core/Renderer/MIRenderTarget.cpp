#include "MIRenderTarget.h"

MIRenderTarget::MIRenderTarget()
	: m_unTargetViewNum(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, m_vpRenderTargetView(nullptr)
	, m_pDepthStencilView(nullptr)
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
{
	
}
