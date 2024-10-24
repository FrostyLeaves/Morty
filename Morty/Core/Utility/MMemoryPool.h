/**
 * @File         MVulkanUniformBufferPool
 * 
 * @Created      2020-07-22 14:36:30
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

struct MORTY_API MemoryInfo {
    size_t begin = 0;
    size_t size  = 0;

    bool   operator<(const MemoryInfo& info) { return begin < info.begin; }

    bool   operator<(const MemoryInfo& info) const { return begin < info.begin; }
};

class MORTY_API MMemoryPool
{

public:
    MMemoryPool(const size_t& nPoolSize);

    virtual ~MMemoryPool();

public:
    bool   AllowMemory(const size_t& unSize, MemoryInfo& info);

    void   FreeMemory(MemoryInfo& info);

    void   ResizeMemory(const size_t& nPoolSize);

    size_t GetMaxMemorySize() const { return m_maxMemorySize; }

private:
    size_t                  m_maxMemorySize = 0;

    std::vector<MemoryInfo> m_freeMemory;
};

}// namespace morty