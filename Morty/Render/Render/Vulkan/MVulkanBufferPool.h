/**
 * @File         MVulkanBufferPool
 * 
 * @Created      2020-07-22 14:36:30
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVULKAN_BUFFERPOOL_H_
#define _M_MVULKAN_BUFFERPOOL_H_
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#include "MMemoryPool.h"
#include "Utility/MIDPool.h"
#include <vector>
#include <map>

class MVulkanDevice;
struct MShaderConstantParam;
class MORTY_API MVulkanBufferPool
{
public:


public:
    MVulkanBufferPool(MVulkanDevice* pDevice);
    virtual ~MVulkanBufferPool();

public:

    bool Initialize();

    void Release();

    bool AllowBufferMemory(MShaderConstantParam* pParam);

	void FreeBufferMemory(MShaderConstantParam* pParam);

    bool AllowReadBackBuffer(const uint32_t& unMemorySize, uint32_t& unBufferID, MemoryInfo& info);
    void FreeReadBackBuffer(const uint32_t& unBufferID);

    void FreeDynamicUniformMemory(MemoryInfo& info);

    MByte* GetReadBackMemory() { return m_pReadBackMemoryMapping; }
    VkBuffer GetReadBackBuffer() { return m_VkReadBackBuffer; }
protected:

	bool AllowUniformBufferMemory(MShaderConstantParam* pParam);

	bool AllowDynamicUniformBufferMemory(MShaderConstantParam* pParam);

    bool AllowDynamicStorageBufferMemory(MShaderConstantParam* pParam);

	void FreeUniformBufferMemory(MShaderConstantParam* pParam);

	void FreeDynamicUniformBufferMemory(MShaderConstantParam* pParam);


private:
    MVulkanDevice* m_pDevice;

    uint32_t m_unMinUboAlignment;
    uint32_t m_unDynamicUniformBufferMemorySize;
    MMemoryPool m_DynamicUniformMemoryPool;
    std::map<MShaderConstantParam*, MemoryInfo> m_tDynamicUniformMemory;

	VkBuffer m_VkDynamicUniformBuffer;
	VkDeviceMemory m_VkDynamicUniformMemory;
	MByte* m_pDynamicUniformMemoryMapping;


    uint32_t m_unReadBackBufferMemorySize;
    MMemoryPool m_ReadBackMemoryPool;
    MRepeatIDPool<uint32_t> m_ReadBackIDPool;
    std::map<uint32_t, MemoryInfo> m_tReadBackMemory;

    VkBuffer m_VkReadBackBuffer;
    VkDeviceMemory m_VkReadBackMemory;

    MByte* m_pReadBackMemoryMapping;
};


#endif


#endif