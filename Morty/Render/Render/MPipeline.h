/**
 * @File         MRenderPipeline
 * 
 * @Created      2022-07-23 22:09:42
 *
 * @Author       DoubleYe
**/

#ifndef _M_MPIPELINE_H_
#define _M_MPIPELINE_H_
#include "Render/MRenderGlobal.h"


class MMaterial;
class MRenderPass;
class MShaderPropertyBlock;

struct MORTY_API MPipelineLayout
{
    VkPipelineLayout vkPipelineLayout;
    std::vector<VkDescriptorSetLayout> vDescriptorSetLayouts;
};

class MORTY_API MPipeline
{
public:
    MPipeline() = default;
    virtual ~MPipeline() = default;

public:
    std::set<std::shared_ptr<MShaderPropertyBlock>> m_tShaderPropertyBlocks = {};

    MPipelineLayout m_pipelineLayout;
    VkPipelineBindPoint m_vkPipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
};

class MORTY_API MGraphicsPipeline : public MPipeline
{
public:


    VkPipeline GetSubpassPipeline(size_t nSubPassIdx);


public:
#if RENDER_GRAPHICS == MORTY_VULKAN
    std::vector<VkPipeline> m_vSubpassPipeline = {};
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


#endif
