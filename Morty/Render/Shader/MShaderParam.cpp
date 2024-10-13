#include "MShaderParam.h"
#include "Basic/MTexture.h"

using namespace morty;

MShaderConstantParam::MShaderConstantParam()
    : MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
#endif
}

MShaderConstantParam::MShaderConstantParam(const MShaderConstantParam& param)
    : MShaderParam(param)
{
    strName     = param.strName;
    var         = MVariant::Clone(param.var);
    eShaderType = param.eShaderType;

    SetDirty();

#if RENDER_GRAPHICS == MORTY_VULKAN
    unSet              = param.unSet;
    unBinding          = param.unBinding;
    m_vkDescriptorType = param.m_vkDescriptorType;

    m_vkBuffer       = VK_NULL_HANDLE;
    m_vkBufferInfo   = {VK_NULL_HANDLE, 0, 0};
    m_vkBufferMemory = VK_NULL_HANDLE;
    m_unMemoryOffset = 0;
    m_memoryMapping  = 0;

    m_unVkMemorySize = param.m_unVkMemorySize;
#endif
}

MShaderTextureParam::MShaderTextureParam()
    : MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#endif
}

void MShaderTextureParam::SetTexture(MTexturePtr pTexture)
{
    if (this->pTexture != pTexture)
    {
        this->pTexture = pTexture;
        SetDirty();
    }
}

std::shared_ptr<MShaderTextureParam> MShaderTextureParam::Clone() const
{
    return std::make_shared<MShaderTextureParam>(*this);
}

MShaderSampleParam::MShaderSampleParam()
    : MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
#endif
}

MShaderStorageParam::MShaderStorageParam()
    : MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
#endif
}

MShaderSubpasssInputParam::MShaderSubpasssInputParam()
    : MShaderTextureParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
#endif
}
