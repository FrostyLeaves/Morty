/**
 * @File         MBuffer
 * 
 * @Created      2022-09-20 15:33:13
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "RHI/Abstract/MBufferRHI.h"
#include "Type/MType.h"

namespace morty
{

class MIDevice;
class MORTY_API MBuffer : public MTypeClass
{
public:
    MORTY_CLASS(MBuffer)

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
                   MBuffer() = default;
                   MBuffer(const MBuffer& other);
    const MBuffer& operator=(const MBuffer& other);
    ~              MBuffer() = default;

    static MBuffer CreateBuffer(MMemoryType memory, uint32_t usage, const char* debugName = nullptr);

    static MBuffer CreateVertexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleVertexBuffer(const char* debugName = nullptr);

    static MBuffer CreateIndexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleIndexBuffer(const char* debugName = nullptr);

    static MBuffer CreateHostVisibleIndirectBuffer(const char* debugName = nullptr);

    static MBuffer CreateIndirectDrawBuffer(const char* debugName = nullptr);

    static MBuffer CreateStorageBuffer(const char* debugName = nullptr);


public:
    void                      ReallocMemory(const size_t& unNewSize);

    size_t                    GetSize() const { return m_unDataSize; }

    void                      GenerateBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);

    void                      UploadBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);

    void                      UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* data, const size_t& size);

    void                      DestroyBuffer(MIDevice* pDevice);

    void                      ResizeBuffer(MIDevice* pDevice, size_t size);

    void                      DownloadBuffer(MIDevice* pDevice, MByte* data, const size_t& size);

    template<typename T> void ApplyData(MIDevice* device, const std::vector<T>& data)
    {
        if (data.size() * sizeof(T) > m_unDataSize)
        {
            ReallocMemory(data.size() * sizeof(T));
            DestroyBuffer(device);
            GenerateBuffer(device, reinterpret_cast<const MByte*>(data.data()), data.size() * sizeof(T));
        }
        else { UploadBuffer(device, reinterpret_cast<const MByte*>(data.data()), data.size() * sizeof(T)); }
    }

    void ApplyData(MIDevice* device, size_t offset, const MByte* data, size_t size)
    {
        if (m_unDataSize < offset + size)
        {
            ReallocMemory(offset + size);
            ResizeBuffer(device, offset + size);
        }

        UploadBuffer(device, offset, data, size);
    }


#if MORTY_DEBUG
    const char* GetDebugName() const { return m_strDebugName.c_str(); }
    MString     m_strDebugName;
#endif

    size_t                      m_unDataSize = 0;
    MMemoryType                 m_memoryType = MMemoryType::EUnknow;
    uint32_t                    m_usageType  = 0;
    MStageType                  m_stageType  = MStageType::EUnknow;


    std::unique_ptr<MBufferRHI> m_bufferRHI = nullptr;
};

}// namespace morty