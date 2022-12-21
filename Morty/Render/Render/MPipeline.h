/**
 * @File         MRenderPipeline
 * 
 * @Created      2022-07-23 22:09:42
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPIPELINE_H_
#define _M_MPIPELINE_H_
#include "Render/MRenderGlobal.h"


class MMaterial;
class MRenderPass;
class MShaderPropertyBlock;

struct MORTY_API MPipelineLayout
{
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> vSetLayouts;
};

class MORTY_API MPipeline
{
public:
    MPipeline() = default;
    virtual ~MPipeline() = default;

public:
    std::shared_ptr<MPipelineLayout> m_pPipelineLayout = nullptr;

    std::vector<std::shared_ptr<MShaderPropertyBlock>> vShaderParamSets = {};

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
#endif 
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


public:

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkPipeline m_vkPipeline = VK_NULL_HANDLE;
#endif
};


#endif
