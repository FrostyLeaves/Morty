/**
 * @File         MVertexBuffer
 * 
 * @Created      2021-07-14 18:55:04
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVERTEXBUFFER_H_
#define _M_MVERTEXBUFFER_H_
#include "Render/MRenderGlobal.h"


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


#endif
