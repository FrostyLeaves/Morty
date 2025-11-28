#include "MShaderParam.h"
#include "Basic/MTexture.h"

using namespace morty;

MShaderUniformParam::MShaderUniformParam()
    : MShaderParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
#endif
}

MShaderUniformParam::MShaderUniformParam(const MShaderUniformParam& param)
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

void MShaderTextureParam::SetTexture(MTexturePtr texture)
{
    if (this->texture != texture)
    {
        this->texture = texture;
        SetDirty();
    }
}

std::unique_ptr<MShaderTextureParam> MShaderTextureParam::Clone() const
{
    return std::make_unique<MShaderTextureParam>(*this);
}

MShaderSamplerParam::MShaderSamplerParam()
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

MShaderSubpassInputParam::MShaderSubpassInputParam()
    : MShaderTextureParam()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
#endif
}
