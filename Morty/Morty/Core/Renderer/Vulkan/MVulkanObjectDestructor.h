/**
 * @File         MVulkanObjectDestructor
 * 
 * @Created      2020-07-07 21:06:34
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANOBJECTDESTRUCTOR_H_
#define _M_MVULKANOBJECTDESTRUCTOR_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"

#include <vector>

class MVulkanDevice;
class MORTY_CLASS MVulkanObjectDestructor
{
public:
    MVulkanObjectDestructor(MVulkanDevice* pDevice);
    ~MVulkanObjectDestructor() {}

public:

    void FrameFinished(const uint32_t& unFrameIndex);
    
    bool Initialize();

    void Release();

public:

	bool GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	bool InitDescriptorPool();
	bool InitSampler();

public:

#define M_VULKAN_DESTROY_LATER_FUNC(VK_TYPE) \
std::vector<Vk##VK_TYPE> m_v##VK_TYPE[2];\
void Destroy##VK_TYPE##Later(Vk##VK_TYPE buffer){\
		m_v##VK_TYPE[m_unSafeIdx].push_back(buffer); \
}\


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
	M_VULKAN_DESTROY_LATER_FUNC(Fence);
	M_VULKAN_DESTROY_LATER_FUNC(Semaphore);
	M_VULKAN_DESTROY_LATER_FUNC(CommandBuffer);
	M_VULKAN_DESTROY_LATER_FUNC(Event);
	M_VULKAN_DESTROY_LATER_FUNC(Sampler);

    std::vector<VkDescriptorSet> m_vDescriptorSets[2];
	void DestroyDescriptorSetLater(VkDescriptorSet vDescriptorSet);

    MVulkanDevice* m_pDevice;

	VkDescriptorPool m_VkDescriptorPool;

	VkSampler m_VkDefaultSampler;
	VkSampler m_VkLessEqualSampler;

	uint32_t m_unSafeIdx;
};


#endif


#endif