/**
 * @File         MVulkanUniformBufferPool
 * 
 * @Created      2020-07-22 14:36:30
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANUNIFORMBUFFERPOOL_H_
#define _M_MVULKANUNIFORMBUFFERPOOL_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"

#include <vector>
#include <map>

class MVulkanDevice;
struct MShaderConstantParam;
class MORTY_CLASS MVulkanUniformBufferPool
{
public:

    struct MemoryInfo
    {
        uint32_t begin;
        uint32_t size;

		bool operator< (const MemoryInfo& info) { return begin < info.begin; }
		bool operator< (const MemoryInfo& info) const { return begin < info.begin; }
    };

public:
    MVulkanUniformBufferPool(MVulkanDevice* pDevice);
    virtual ~MVulkanUniformBufferPool();

public:

    bool Initialize();

    void Release();

    bool AllowBufferMemory(MShaderConstantParam* pParam);

	void FreeBufferMemory(MShaderConstantParam* pParam);

protected:

	bool AllowUniformBufferMemory(MShaderConstantParam* pParam);

	bool AllowDynamicUniformBufferMemory(MShaderConstantParam* pParam);

	void FreeUniformBufferMemory(MShaderConstantParam* pParam);

	void FreeDynamicUniformBufferMemory(MShaderConstantParam* pParam);


    bool AllowMemory(const uint32_t& unSize, MemoryInfo& info);

    void FreeMemory(MemoryInfo& info);

private:
    MVulkanDevice* m_pDevice;

    uint32_t m_unMinUboAlignment;
    uint32_t m_unBufferMemorySize;

    std::vector<MemoryInfo> m_vFreeMemory;
    std::map<MShaderConstantParam*, MemoryInfo> m_tUsingMemory[M_BUFFER_NUM];

    VkBuffer m_VkBuffer;
    VkDeviceMemory m_VkDeviceMemory;

    MByte* m_pMemoryMapping;
};


#endif


#endif