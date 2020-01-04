#include "MIRenderTarget.h"

MIRenderTarget::MIRenderTarget()
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	: m_pTargetView(nullptr)
	, m_pDepthStencilView(nullptr)
#endif
{

}
