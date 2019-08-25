#include "MVertex.h"
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "MDirectX11Renderer.h"
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif

void MVertexLayout::SetUseLayout(MIRenderer* pRenderer)
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
}
