/**
 * @File         MShaderBuffer
 * 
 * @Created      2020-07-20 14:41:06
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Shader/MShaderPropertyBlock.h"

namespace morty
{

class MORTY_API MShaderBuffer
{
public:
    MShaderBuffer();

    virtual ~MShaderBuffer() = default;

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_shaderSets;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkShaderModule                  m_vkShaderModule = VK_NULL_HANDLE;
    VkPipelineShaderStageCreateInfo m_vkShaderStageInfo;
#endif
};


class MVertexShaderBuffer : public MShaderBuffer
{
#if RENDER_GRAPHICS == MORTY_VULKAN
public:
    std::vector<VkVertexInputAttributeDescription> m_attributeDescs;
    std::vector<VkVertexInputBindingDescription>   m_bindingDescs;
#endif
};

class MPixelShaderBuffer : public MShaderBuffer
{
};

class MComputeShaderBuffer : public MShaderBuffer
{
};

class MGeometryShaderBuffer : public MShaderBuffer
{
};

}// namespace morty