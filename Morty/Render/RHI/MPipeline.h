/**
 * @File         MRenderPipeline
 * 
 * @Created      2022-07-23 22:09:42
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

namespace morty
{

class MMaterial;
class MRenderPass;
class MShaderParameterSet;
struct MORTY_API MPipelineLayout {
    VkPipelineLayout                   vkPipelineLayout;
    std::vector<VkDescriptorSetLayout> vDescriptorSetLayouts;
    uint32_t                           requiredSetsMask = 0;
};

class MORTY_API MPipeline
{
public:
    MPipeline() = default;

    virtual ~MPipeline() = default;

public:
    std::set<std::shared_ptr<MShaderParameterSet>> m_shaderParameterSets = {};

    MPipelineLayout                                m_pipelineLayout;
    VkPipelineBindPoint                            m_vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
};

class MORTY_API MGraphicsPipeline : public MPipeline
{
public:
    VkPipeline GetSubpassPipeline(size_t nSubPassIdx) const;


public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    std::vector<VkPipeline> m_subpassPipeline = {};
#endif
};


class MORTY_API MComputePipeline : public MPipeline
{
public:
    MComputePipeline();

public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    VkPipeline m_vkPipeline = VK_NULL_HANDLE;
#endif
};

}// namespace morty