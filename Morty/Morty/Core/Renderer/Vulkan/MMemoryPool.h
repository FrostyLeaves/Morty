/**
 * @File         MVulkanUniformBufferPool
 * 
 * @Created      2020-07-22 14:36:30
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMEMORY_POOL_H_
#define _M_MMEMORY_POOL_H_
#include "MGlobal.h"

#include <map>
#include <vector>

struct MORTY_API MemoryInfo
{
	uint32_t begin;
	uint32_t size;

	bool operator< (const MemoryInfo& info) { return begin < info.begin; }
	bool operator< (const MemoryInfo& info) const { return begin < info.begin; }
};

class MORTY_API MMemoryPool
{
public:

    

public:
    MMemoryPool(const uint32_t& unPoolSize);
    virtual ~MMemoryPool();

public:

    bool AllowMemory(const uint32_t& unSize, MemoryInfo& info);

    void FreeMemory(MemoryInfo& info);

private:
    uint32_t m_unBufferMemorySize;

    std::vector<MemoryInfo> m_vFreeMemory;
};


#endif