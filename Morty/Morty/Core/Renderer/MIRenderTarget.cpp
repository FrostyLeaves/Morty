#include "MIRenderTarget.h"

MIRenderTarget::MIRenderTarget()
	: m_backgroundColor(0.0f, 0.0f, 0.0f, 1.0f)
	, m_unTargetViewNum(0)
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, m_vpTargetView(nullptr)
	, m_pDepthStencilView(nullptr)
#endif
{
	
}
