#include "MShaderBuffer.h"


MShaderBuffer::MShaderBuffer()
	: m_MaterialSet(0)
	, m_FrameSet(1)
	, m_MeshSet(2)
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_VkShaderModule = VK_NULL_HANDLE;
#endif

}

MVertexShaderBuffer::MVertexShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexShader = nullptr;
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

MPixelShaderBuffer::MPixelShaderBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pPixelShader = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}
