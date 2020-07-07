/**
 * @File         MVulkanBufferManager
 * 
 * @Created      2020-07-07 21:06:34
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANBUFFERMANAGER_H_
#define _M_MVULKANBUFFERMANAGER_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

#include <vector>

class MVulkanDevice;
class MORTY_CLASS MVulkanBufferManager
{
public:
    MVulkanBufferManager(MVulkanDevice* pDevice);
    ~MVulkanBufferManager() {}

public:

    void FrameFinished(const uint32_t& unFrameIndex);
    
public:

	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void DestroyBufferLater(const uint32_t& unFrameIndex, VkBuffer& buffer);

    void DestroyDeviceMemoryLater(const uint32_t& unFrameIndex, VkDeviceMemory& memory);

    void DestroyFrameBufferLater(const uint32_t& unFrameIndex, VkFramebuffer& buffer);

    void DestroyImageLater(const uint32_t& unFrameIndex, VkImage& image);

    void DestroyImageViewLater(const uint32_t& unFrameIndex, VkImageView& view);

    void DestroyRenderPassLater(const uint32_t& unFrameIndex, VkRenderPass& pass);

private:

    MVulkanDevice* m_pDevice;


    std::vector<VkBuffer> m_vBuffer[M_BUFFER_NUM];
    std::vector< VkFramebuffer> m_vFrameBuffer[M_BUFFER_NUM];
    std::vector<VkDeviceMemory> m_vDeviceMemory[M_BUFFER_NUM];
	std::vector< VkImageView> m_vImageView[M_BUFFER_NUM];
	std::vector< VkImage> m_vImage[M_BUFFER_NUM];
    std::vector<VkRenderPass> m_vRenderPass[M_BUFFER_NUM];
};


#endif


#endif