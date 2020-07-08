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

public:

#define M_VULKAN_DESTROY_LATER_FUNC(VK_TYPE) \
std::vector<Vk##VK_TYPE> m_v##VK_TYPE[M_BUFFER_NUM];\
void Destroy##VK_TYPE##Later(const uint32_t& unFrameIndex, Vk##VK_TYPE& buffer){m_v##VK_TYPE[unFrameIndex].push_back(buffer);}\


	M_VULKAN_DESTROY_LATER_FUNC(Buffer);
	M_VULKAN_DESTROY_LATER_FUNC(Framebuffer);
	M_VULKAN_DESTROY_LATER_FUNC(DeviceMemory);
	M_VULKAN_DESTROY_LATER_FUNC(ImageView);
	M_VULKAN_DESTROY_LATER_FUNC(Image);
	M_VULKAN_DESTROY_LATER_FUNC(RenderPass);
    M_VULKAN_DESTROY_LATER_FUNC(PipelineLayout);
	M_VULKAN_DESTROY_LATER_FUNC(DescriptorSetLayout);
	M_VULKAN_DESTROY_LATER_FUNC(ShaderModule);
	M_VULKAN_DESTROY_LATER_FUNC(Pipeline);

    std::vector<std::vector<VkDescriptorSet>> m_vDescriptorSets[M_BUFFER_NUM];
    void DestroyDescriptorSets(const uint32_t& unFrameIndex, std::vector<VkDescriptorSet>& vDescriptorSets);

    MVulkanDevice* m_pDevice;
};


#endif


#endif