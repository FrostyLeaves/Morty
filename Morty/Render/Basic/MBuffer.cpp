#include "MBuffer.h"
#include "RHI/Abstract/MIDevice.h"
#include <stdint.h>

using namespace morty;

MBuffer::MBuffer() {}

MBuffer::~MBuffer() {}

MBuffer MBuffer::CreateBuffer(MMemoryType memory, uint32_t usage, const char* debugName)
{
    MBuffer buffer;
    buffer.m_memoryType = memory;
    buffer.m_usageType  = usage;

#if MORTY_DEBUG
    if (debugName) { buffer.m_strDebugName = debugName; }
#else
    MORTY_UNUSED(debugName);
#endif

    return buffer;
}

MBuffer MBuffer::CreateVertexBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EDeviceLocal, MUsageType::EVertex, debugName);
}

MBuffer MBuffer::CreateHostVisibleVertexBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EHostVisible, MUsageType::EVertex, debugName);
}

MBuffer MBuffer::CreateIndexBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EDeviceLocal, MUsageType::EIndex, debugName);
}

MBuffer MBuffer::CreateHostVisibleIndexBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EHostVisible, MUsageType::EIndex, debugName);
}

MBuffer MBuffer::CreateHostVisibleIndirectBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EHostVisible, MUsageType::EIndirect, debugName);
}

MBuffer MBuffer::CreateIndirectDrawBuffer(const char* debugName)
{
    return CreateBuffer(
            MMemoryType::EDeviceLocal,
            MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect,
            debugName
    );
}

MBuffer MBuffer::CreateStorageBuffer(const char* debugName)
{
    return CreateBuffer(MMemoryType::EHostVisible, MUsageType::EStorage, debugName);
}

const MBuffer& MBuffer::operator=(const MBuffer& other)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    MORTY_ASSERT(m_vkBuffer == VK_NULL_HANDLE);
    MORTY_ASSERT(m_vkDeviceMemory == VK_NULL_HANDLE);
#endif

    m_memoryType = other.m_memoryType;
    m_usageType  = other.m_usageType;

    m_stageType = MStageType::EWaitAllow;

    return *this;
}

void MBuffer::ReallocMemory(const size_t& unNewSize)
{
    m_unDataSize = unNewSize;
    m_stageType  = MBuffer::MStageType::EWaitAllow;
}

void MBuffer::GenerateBuffer(MIDevice* pDevice, const MByte* data, const size_t& size)
{
    pDevice->GenerateBuffer(this, data, size);
}

void MBuffer::UploadBuffer(MIDevice* pDevice, const MByte* data, const size_t& size)
{
    pDevice->UploadBuffer(this, 0, data, size);
}

void MBuffer::UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* data, const size_t& size)
{
    pDevice->UploadBuffer(this, nBeginOffset, data, size);
}

void MBuffer::DestroyBuffer(MIDevice* pDevice) { pDevice->DestroyBuffer(this); }

void MBuffer::DownloadBuffer(MIDevice* pDevice, MByte* data, const size_t& size)
{
    pDevice->DownloadBuffer(this, data, size);
}
