#include "MVertexBuffer.h"

using namespace morty;

MVertexBuffer::MVertexBuffer()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkVertexBuffer       = VK_NULL_HANDLE;
    m_vkVertexBufferMemory = VK_NULL_HANDLE;
    m_vkIndexBuffer        = VK_NULL_HANDLE;
    m_vkIndexBufferMemory  = VK_NULL_HANDLE;
#endif
}
