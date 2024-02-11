/**
 * @File         MVertexBuffer
 * 
 * @Created      2021-07-14 18:55:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

MORTY_SPACE_BEGIN

class MORTY_API MVertexBuffer
{
public:
	MVertexBuffer();
	~MVertexBuffer() {}

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;
	VkBuffer m_VkIndexBuffer;
	VkDeviceMemory m_VkIndexBufferMemory;
#endif
};

MORTY_SPACE_END