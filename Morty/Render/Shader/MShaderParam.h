/**
 * @File         MRenderPass
 *
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MTexture.h"
#include "Variant/MVariant.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanWrapper.h"

#endif

namespace morty
{

enum class MESamplerType
{
    ENearest,
    ELinear,
};

enum class MESamplerFormat
{
    EFloat = 0,
    EInt   = 1,
};

class MBuffer;
struct MORTY_API MShaderParam {
public:
    MShaderParam() = default;

    virtual ~MShaderParam() = default;

    MStringId strName;
    uint32_t  eShaderType = 0;

    bool      bDirty = true;

    void      SetDirty() { bDirty = true; }

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkDescriptorType m_vkDescriptorType;
    uint32_t         unSet     = 0;
    uint32_t         unBinding = 0;
#endif
};

struct MORTY_API MShaderConstantParam : public MShaderParam {
    MShaderConstantParam();

    MShaderConstantParam(const MShaderConstantParam& param);

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkBuffer               m_vkBuffer       = VK_NULL_HANDLE;
    VkDeviceMemory         m_vkBufferMemory = VK_NULL_HANDLE;
    VkDescriptorBufferInfo m_vkBufferInfo   = {VK_NULL_HANDLE, 0, 0};
    uint32_t               m_unMemoryOffset = 0;
    MByte*                 m_memoryMapping  = 0;
    uint32_t               m_unVkMemorySize = 0;
#endif

    MVariant var;
};


struct MShaderTextureParam : public MShaderParam {
    MShaderTextureParam();

public:
    virtual void                                 SetTexture(MTexturePtr pTexture);

    virtual MTexturePtr                          GetTexture() { return pTexture; }

    virtual std::shared_ptr<MShaderTextureParam> Clone() const;

public:
    MTexturePtr     pTexture    = nullptr;
    void*           pImageIdent = nullptr;
    METextureType   eType       = METextureType::ETexture2D;
    MESamplerFormat eFormat     = MESamplerFormat::EFloat;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkDescriptorImageInfo m_vkImageInfo = {};
#endif
};

struct MShaderStorageParam : public MShaderParam {
    MShaderStorageParam();

public:
    const MBuffer* pBuffer     = nullptr;
    bool           bWritable   = false;
    void*          pImageIdent = nullptr;

    void           SetBuffer(const MBuffer* buf) { pBuffer = buf; }

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkDescriptorBufferInfo m_vkBufferInfo = {VK_NULL_HANDLE, 0, 0};
#endif
};

struct MShaderSubpasssInputParam : public MShaderTextureParam {
    MShaderSubpasssInputParam();
};

struct MShaderSampleParam : public MShaderParam {
    MShaderSampleParam();

    MESamplerType eSamplerType = MESamplerType::ELinear;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkSampler m_vkSampler = VK_NULL_HANDLE;
#endif
};

}// namespace morty