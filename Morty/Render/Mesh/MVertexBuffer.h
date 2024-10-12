/**
 * @File         MVertexBuffer
 * 
 * @Created      2021-07-14 18:55:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

namespace morty
{

class MORTY_API MVertexBuffer
{
public:
    MVertexBuffer();

    ~MVertexBuffer() {}

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkBuffer       m_vkVertexBuffer;
    VkDeviceMemory m_vkVertexBufferMemory;
    VkBuffer       m_vkIndexBuffer;
    VkDeviceMemory m_vkIndexBufferMemory;
#endif
};

}// namespace morty