/**
 * @File         MVulkanPipelineManager
 * 
 * @Created      2020-06-23 12:16:06
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Utility/MIDPool.h"
#include "Render/MRenderPass.h"
#include "Render/MPipeline.h"

class MMaterialTemplate;
class MShaderProgram;
class MVulkanDevice;
class MShaderPropertyBlock;
class MComputeDispatcher;
struct MShaderTextureParam;
struct MShaderConstantParam;
struct MShaderStorageParam;

class MORTY_API MVulkanPipelineManager
{
public:

    struct MORTY_API MPipelineKey
    {
        const std::shared_ptr<const MShaderProgram> pShaderProgram = nullptr;
        const MRenderPass* pRenderPass = nullptr;

        MPipelineKey(const std::shared_ptr<const MShaderProgram> _pShaderProgram, const MRenderPass* _pRenderPass)
            : pShaderProgram(_pShaderProgram)
            , pRenderPass(_pRenderPass)
        {}

        bool operator == (const MPipelineKey& other) const {
            return pShaderProgram == other.pShaderProgram && pRenderPass == other.pRenderPass;
        }

        bool operator == (const std::shared_ptr<MShaderProgram>& _pShaderProgram) const {
            return pShaderProgram == _pShaderProgram;
        }

        bool operator == (const MRenderPass* _pRenderPass) const {
            return pRenderPass == _pRenderPass;
        }

        bool operator < (const MPipelineKey& other) const {
            if (pShaderProgram < other.pShaderProgram)
                return true;
            else if (pShaderProgram == other.pShaderProgram)
                return pRenderPass < other.pRenderPass;
            return false;
        }
    };

    struct MORTY_API MPipelineLayout
    {
        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> vDescriptorSetLayouts = {};
    };


public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);
    virtual ~MVulkanPipelineManager();

    void Release();

public:

    std::shared_ptr<MGraphicsPipeline> FindOrCreateGraphicsPipeline(const MMaterialTemplate* pMaterial, const MRenderPass* pRenderPass);
    std::shared_ptr<MComputePipeline> FindOrCreateComputePipeline(MComputeDispatcher* pComputeDispatcher);

public:

    void DestroyRenderPass(MRenderPass* pRenderPass);
    void DestroyPipeline(const std::shared_ptr<MPipeline>& pPipeline);
    void DestroyPipelineLayout(const std::shared_ptr<MPipeline>& pPipeline);
    void DestroyGraphicsPipeline(const std::shared_ptr<MGraphicsPipeline>& pGraphicsPipeline);
    void DestroyComputePipeline(const std::shared_ptr<MComputePipeline>& pComputePipeline);

    VkPipeline CreateGraphicsPipeline(const std::shared_ptr<MPipeline>& pPipeline, const MMaterialTemplate* pMaterial, const MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);
    VkPipeline CreateComputePipeline(const std::shared_ptr<MPipeline>& pPipeline, MComputeDispatcher* pComputeDispatcher);

    void AllocateShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock, const std::shared_ptr<MPipeline>& pPipeline);
    void DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock);

    void DestroyShaderPropertyBlockImpl(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock) const;

public:

    void GeneratePipelineLayout(const std::shared_ptr<MPipeline>& pPipeline, const std::shared_ptr<MShaderProgram>& pShaderProgram);

    void BindConstantParam(const std::shared_ptr<MShaderConstantParam> pParam, VkWriteDescriptorSet& writeDescriptorSet);
    void BindTextureParam(const std::shared_ptr<MShaderTextureParam> pParam, VkWriteDescriptorSet& writeDescriptorSet);
    void BindStorageParam(const std::shared_ptr<MShaderStorageParam> pParam, VkWriteDescriptorSet& writeDescriptorSet);



private:

    std::map<MPipelineKey, std::shared_ptr<MPipeline>> m_tPipelineTable;

    MVulkanDevice* m_pDevice;

};


#endif
