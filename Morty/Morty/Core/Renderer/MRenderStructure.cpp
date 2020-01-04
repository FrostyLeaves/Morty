#include "MRenderStructure.h"
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "MDirectX11Renderer.h"
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif


MVertexBuffer::MVertexBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
}

MShaderBuffer::MShaderBuffer()
{

}

MVertexShaderBuffer::MVertexShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexShader = nullptr;
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES
#endif
}

MPixelShaderBuffer::MPixelShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pPixelShader = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES
#endif
}

MInputLayout::MInputLayout()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES
#endif
}

MTextureBuffer::MTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif

}

MDepthTextureBuffer::MDepthTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
	m_pDepthStencilView = nullptr;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif

}

MTextureBuffer::~MTextureBuffer()
{

}
