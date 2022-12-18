/**
 * @File         MVulkanPipelineManager
 * 
 * @Created      2020-06-23 12:16:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVULKANPIPELINEMANAGER_H_
#define _M_MVULKANPIPELINEMANAGER_H_
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Utility/MIDPool.h"
#include "Render/MRenderPass.h"

class MShaderGroup;
class MMaterial;
class MVulkanDevice;
class MShaderParamSet;
class MComputeDispatcher;
struct MShaderTextureParam;
struct MShaderConstantParam;
struct MShaderStorageParam;

struct MRenderPassPipelines
{
    std::vector<VkPipeline> vSubpassPipeline;
};

struct MPipelineLayout
{
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> vSetLayouts;

};

struct MMaterialPipelineGroup
{
    MPipelineLayout layouts;

    std::vector<MShaderParamSet*> vShaderParamSets;

    std::map<uint32_t, MRenderPassPipelines> vRenderPassPipeline;
};

struct MComputeDispatcherData
{
    MPipelineLayout layouts;

    VkPipeline vkPipeline;
};

class MORTY_API MVulkanPipelineManager
{
public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);
    virtual ~MVulkanPipelineManager();

    void Release();

public:

    VkPipeline FindOrCreateGraphicsPipeline(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, const uint32_t& unSubpassIdx);
    VkPipeline FindOrCreateComputePipeline(MComputeDispatcher* pComputeDispatcher);

	std::shared_ptr<MMaterialPipelineGroup> FindOrCreatePipelineGroup(std::shared_ptr<MMaterial> pMaterial);
    std::shared_ptr<MMaterialPipelineGroup> FindPipelineGroup(const uint32_t& nMaterialID) const;

public:
	bool RegisterMaterial(std::shared_ptr<MMaterial> pMaterial);
	bool UnRegisterMaterial(std::shared_ptr<MMaterial> pMaterial);

    void RegisterRenderPass(MRenderPass* pRenderPass);
    void UnRegisterRenderPass(MRenderPass* pRenderPass);

    bool RegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher);
    bool UnRegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher);

public:

	void BindConstantParam(MShaderConstantParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    void BindTextureParam(MShaderTextureParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    void BindStorageParam(MShaderStorageParam* pParam, VkWriteDescriptorSet& writeDescriptorSet);

    MPipelineLayout CreateMaterialPipelineGroup(const MShaderGroup* pShaderGroup, VkShaderStageFlags vkShaderStageFlags);

    void DestroyMaterialPipelineLayout(const std::shared_ptr<MMaterialPipelineGroup>& pLayoutData) const;
    void DestroyMaterialPipeline(const std::shared_ptr<MMaterialPipelineGroup>& pLayoutData) const;

    VkPipeline CreateGraphicsPipeline(std::shared_ptr<MMaterial> pMaterial, VkPipelineLayout pipelineLayout, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);
    VkPipeline CreateComputePipeline(MComputeDispatcher* pComputeDispatcher, VkPipelineLayout pipelineLayout);

    void GenerateShaderParamSet(MShaderParamSet* pParamSet);
    void DestroyShaderParamSet(MShaderParamSet* pParamSet);
    void AllocateShaderParamSet(MShaderParamSet* pParamSet);

    void DestroyShaderParamSetImpl(MShaderParamSet* pParamSet) const;

private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;
    MRepeatIDPool<uint32_t> m_RenderPassIDPool;
    MRepeatIDPool<uint32_t> m_ComputeDispatcherIDPool;

    std::vector<std::shared_ptr<MMaterialPipelineGroup>> m_tPipelineTable;

	std::map<uint32_t, std::shared_ptr<MMaterial>> m_tMaterialMap;

    std::vector<std::shared_ptr<MComputeDispatcherData>> m_tComputeDispatcherData;

    std::map<uint32_t, MComputeDispatcher*> m_tComputeDispatcherMap;

    MVulkanDevice* m_pDevice;

};


#endif

#endif