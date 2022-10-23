/**
 * @File         MBuffer
 * 
 * @Created      2022-09-20 15:33:13
 *
 * @Author       Pobrecito
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

    enum class MUsageType
    {
	    EUnknow = 0,
        EVertex = 1,
        EIndex = 2,
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


    const MBuffer& operator=(const MBuffer& other);

public:

    void ReallocMemory(const size_t& unNewSize); 
    size_t GetSize() const { return m_data.size(); }
    MByte* GetData() { return m_data.data(); }
    const MByte* GetData() const { return m_data.data(); }

    void GenerateBuffer(MIDevice* pDevice);
    void UploadBuffer(MIDevice* pDevice);
    void DestroyBuffer(MIDevice* pDevice);

public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    VkBuffer m_VkBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_VkDeviceMemory = VK_NULL_HANDLE;
#endif
#if _DEBUG
    MString m_strDebugBufferName = "";
#endif

    std::vector<MByte> m_data = {};

    MMemoryType m_eMemoryType = MMemoryType::EUnknow;
    MUsageType m_eUsageType = MUsageType::EUnknow;
    MStageType m_eStageType = MStageType::EUnknow;
};


#endif
