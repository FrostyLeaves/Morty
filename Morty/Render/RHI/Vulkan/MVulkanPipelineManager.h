/**
 * @File         MVulkanPipelineManager
 * 
 * @Created      2020-06-23 12:16:06
 *
 * @Author       DoubleYe
**/

#pragma once

#include <utility>

#include "Utility/MGlobal.h"
#include "Shader/MShader.h"
#include "Shader/MShaderParam.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/MPipeline.h"
#include "RHI/MRenderPass.h"
#include "Utility/MIDPool.h"

namespace morty
{

class MMaterialPass;
class MMaterialTemplate;
class IShaderProgram;
class MVulkanDevice;
class MShaderParameterSet;
class MComputeDispatcher;
struct MShaderTextureParam;
struct MShaderUniformParam;
struct MShaderStorageParam;

class MORTY_API MVulkanPipelineManager
{
public:
    struct MORTY_API MPipelineKey {
        const IShaderProgram* pShaderProgram = nullptr;
        const MRenderPass*    pRenderPass    = nullptr;

        MPipelineKey(const IShaderProgram* shaderProgram, const MRenderPass* renderPass)
            : pShaderProgram(shaderProgram)
            , pRenderPass(renderPass)
        {}

        bool operator==(const MPipelineKey& other) const
        {
            return pShaderProgram == other.pShaderProgram && pRenderPass == other.pRenderPass;
        }

        bool operator==(const IShaderProgram* _pShaderProgram) const { return pShaderProgram == _pShaderProgram; }

        bool operator==(const MRenderPass* _pRenderPass) const { return pRenderPass == _pRenderPass; }

        bool operator<(const MPipelineKey& other) const
        {
            if (pShaderProgram != other.pShaderProgram) return pShaderProgram < other.pShaderProgram;
            return pRenderPass < other.pRenderPass;
        }
    };

    struct MORTY_API MPipelineLayout {
        VkPipelineLayout                   vkPipelineLayout      = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> vDescriptorSetLayouts = {};
    };


public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);

    virtual ~MVulkanPipelineManager();

    void Release();

public:
    std::shared_ptr<MGraphicsPipeline>
    FindOrCreateGraphicsPipeline(const MMaterialPass* materialPass, const MRenderPass* pRenderPass);

    std::shared_ptr<MComputePipeline> FindOrCreateComputePipeline(MComputeDispatcher* pComputeDispatcher);

public:
    void       DestroyRenderPass(MRenderPass* pRenderPass);

    void       DestroyPipeline(const std::shared_ptr<MPipeline>& pPipeline);

    void       DestroyPipelineLayout(const std::shared_ptr<MPipeline>& pPipeline);

    void       DestroyGraphicsPipeline(const std::shared_ptr<MGraphicsPipeline>& pGraphicsPipeline);

    void       DestroyComputePipeline(const std::shared_ptr<MComputePipeline>& pComputePipeline);

    VkPipeline CreateGraphicsPipeline(
            const std::shared_ptr<MPipeline>& pPipeline,
            const MMaterialPass*              materialPass,
            const MRenderPass*                pRenderPass,
            const uint32_t&                   nSubpassIdx
    );

    VkPipeline
         CreateComputePipeline(const std::shared_ptr<MPipeline>& pPipeline, MComputeDispatcher* pComputeDispatcher);

    void AllocateShaderParameterSet(MShaderParameterSet* pParameterSet, const MPipeline* pPipeline);

    void DestroyShaderParameterSet(MShaderParameterSet* pParameterSet);

    void DestroyShaderParameterSetImpl(MShaderParameterSet* pParameterSet) const;

public:
    void        GeneratePipelineLayout(const std::shared_ptr<MPipeline>& pPipeline, IShaderProgram* shaderProgram);

    void        BindConstantParam(const MShaderUniformParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    void        BindTextureParam(MShaderTextureParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    void        BindStorageParam(MShaderStorageParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    MTexturePtr GetDefaultTexture(MShaderTextureParam* pParam);

    const MBuffer* GetDefaultBuffer(MShaderStorageParam* pParam);

private:
    std::map<MPipelineKey, std::shared_ptr<MPipeline>>               m_pipelineTable;

    MVulkanDevice*                                                   m_device;

    std::map<std::pair<MESamplerFormat, METextureType>, MTexturePtr> m_defaultTexture;
    std::map<bool, std::shared_ptr<MBuffer>>                         m_defaultBuffer; // key: bWritable
};

}// namespace morty

#endif
