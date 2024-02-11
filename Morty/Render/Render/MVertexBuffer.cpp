#include "Render/MVertexBuffer.h"

using namespace morty;

MVertexBuffer::MVertexBuffer()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkVertexBuffer = VK_NULL_HANDLE;
	m_VkVertexBufferMemory = VK_NULL_HANDLE;
	m_VkIndexBuffer = VK_NULL_HANDLE;
	m_VkIndexBufferMemory = VK_NULL_HANDLE;
#endif
}
