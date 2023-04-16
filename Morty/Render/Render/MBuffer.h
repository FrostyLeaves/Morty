/**
 * @File         MBuffer
 * 
 * @Created      2022-09-20 15:33:13
 *
 * @Author       DoubleYe
**/

#ifndef _M_MBUFFER_H_
#define _M_MBUFFER_H_
#include "Render/MRenderGlobal.h"


class MIDevice;
class MORTY_API MBuffer
{
public:
    enum class MMemoryType
    {
        EUnknow = 0,
        EHostVisible = 1,
        EDeviceLocal = 2,
    };

    struct MUsageType
    {
        static const uint32_t EUnknow = 0;
        static const uint32_t EVertex = 1;
        static const uint32_t EIndex = 2;
        static const uint32_t EStorage = 4;
        static const uint32_t EUniform = 8;
        static const uint32_t EIndirect = 16;
    };

    enum class MStageType
    {
        EUnknow = 0,
        EWaitAllow = 1,
	    EWaitSync = 2,
        ESynced = 3,
    };


public:
    MBuffer();
    ~MBuffer();


    static MBuffer CreateVertexBuffer();
    static MBuffer CreateHostVisibleVertexBuffer();
    static MBuffer CreateIndexBuffer();
    static MBuffer CreateHostVisibleIndexBuffer();


public:

    const MBuffer& operator=(const MBuffer& other);

public:

    void ReallocMemory(const size_t& unNewSize); 
    size_t GetSize() const { return m_unDataSize; }

    void GenerateBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);
    void UploadBuffer(MIDevice* pDevice, const MByte* data, const size_t& size);
    void UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* data, const size_t& size);
    void DestroyBuffer(MIDevice* pDevice);

public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    VkBuffer m_VkBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VkDeviceMemory = VK_NULL_HANDLE;
#endif
#if MORTY_DEBUG
    MString m_strDebugBufferName = "";
#endif

    MByte* m_mappingData = nullptr;

    size_t m_unDataSize = 0;

    MMemoryType m_eMemoryType = MMemoryType::EUnknow;
    uint32_t m_eUsageType = 0;
    MStageType m_eStageType = MStageType::EUnknow;
};


#endif
