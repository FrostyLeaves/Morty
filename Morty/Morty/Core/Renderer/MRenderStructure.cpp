#include "MRenderStructure.h"

#include "MFunction.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "MDirectX11Renderer.h"
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

MVertexBuffer::MVertexBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif
}

MInputLayout::MInputLayout()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pInputLayout = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif
}

MTextureBuffer::MTextureBuffer()
{
	m_unMipmaps = 1;
	m_unWidth = 0;
	m_unHeight = 0;
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN
	m_VkTextureFormat = VK_FORMAT_R8G8B8A8_SRGB;
	m_VkTextureImage = VK_NULL_HANDLE;
	m_VkImageLayout = VK_IMAGE_LAYOUT_GENERAL;
	m_VkTextureImageMemory = VK_NULL_HANDLE;
	m_VkImageView = VK_NULL_HANDLE;
	m_VkSampler = VK_NULL_HANDLE;
#endif

}

// void* MTextureBuffer::GetResourceView()
// {
// #if RENDER_GRAPHICS == MORTY_DIRECTX_11
// 	return m_pShaderResourceView;
// #elif RENDER_GRAPHICS == MORTY_VULKAN
// 	return nullptr;
// #else
// 	return nullptr;
// #endif
// }


MTextureBuffer::~MTextureBuffer()
{

}

MRenderTextureBuffer::MRenderTextureBuffer()
	: MTextureBuffer()



#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	, m_pRenderTargetView(nullptr)
#elif RENDER_GRAPHICS == MORTY_VULKAN
	//, m_VkFrameBuffer(VK_NULL_HANDLE)
#endif
{
}

MDepthTextureBuffer::MDepthTextureBuffer()
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	m_pTextureBuffer = nullptr;
	m_pShaderResourceView = nullptr;
	m_pDepthStencilView = nullptr;
#elif RENDER_GRAPHICS == MORTY_VULKAN

#endif

}
