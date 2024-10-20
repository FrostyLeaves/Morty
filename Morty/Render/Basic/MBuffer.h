/**
 * @File         MBuffer
 * 
 * @Created      2022-09-20 15:33:13
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

namespace morty
{

class MIDevice;
class MORTY_API MBuffer
{
public:
    enum class MMemoryType
    {
        EUnknow      = 0,
        EHostVisible = 1,
        EDeviceLocal = 2,
    };

    struct MUsageType {
        static const uint32_t EUnknow   = 0;
        static const uint32_t EVertex   = 1;
        static const uint32_t EIndex    = 2;
        static const uint32_t EStorage  = 4;
        static const uint32_t EUniform  = 8;
        static const uint32_t EIndirect = 16;
    };

    enum class MStageType
    {
        EUnknow    = 0,
        EWaitAllow = 1,
        EWaitSync  = 2,
        ESynced    = 3,
    };


public:
    MBuffer();

    ~MBuffer();

    static MBuffer CreateBuffer(MMemoryType memory, uint32_t usage, const char* debugName = nullptr);

    static MBuffer CreateVertexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleVertexBuffer(const char* debugName = nullptr);

    static MBuffer CreateIndexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleIndexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleIndirectBuffer(const char* debugName = nullptr);

    static MBuffer CreateIndirectDrawBuffer(const char* debugName = nullptr);

    static MBuffer CreateStorageBuffer(const char* debugName = nullptr);


public:
    MBuffer(const MBuffer& other) = default;

    const MBuffer& operator=(const MBuffer& other);

public:
    void   ReallocMemory(const size_t& unNewSize);

    size_t GetSize() const { return m_unDataSize; }

    void   GenerateBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);

    void   UploadBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);

    void   UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* data, const size_t& size);

    void   DestroyBuffer(MIDevice* pDevice);

    void   DownloadBuffer(MIDevice* pDevice, MByte* data, const size_t& size);


public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    VkBuffer       m_vkBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory m_vkDeviceMemory = VK_NULL_HANDLE;
#endif
#if MORTY_DEBUG

    const char* GetDebugName() const { return m_strDebugName.c_str(); }

    MString     m_strDebugName;
#endif

    size_t      m_unDataSize = 0;

    MMemoryType m_memoryType = MMemoryType::EUnknow;
    uint32_t    m_usageType  = 0;
    MStageType  m_stageType  = MStageType::EUnknow;
};

}// namespace morty