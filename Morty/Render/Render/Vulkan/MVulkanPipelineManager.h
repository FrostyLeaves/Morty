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

struct MMaterialPipelineGroup
{
    std::vector<MRenderPassPipelines*> vRenderPassGroup;
};

struct MMaterialPipelineLayoutData
{
    MMaterialPipelineLayoutData();

    std::shared_ptr<MMaterial> pMaterial;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> vSetLayouts;
    std::vector<MShaderParamSet*> vShaderParamSets;
};

struct MComputeDispatcherData
{
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

	MMaterialPipelineLayoutData* FindOrCreatePipelineLayout(std::shared_ptr<MMaterial> pMaterial);
	MMaterialPipelineLayoutData* FindPipelineLayout(const uint32_t& nMaterialIdx);

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

    MMaterialPipelineLayoutData* CreateMaterialPipelineLayout(std::shared_ptr<MMaterial> pMaterial);
    void DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData* pLayoutData);

    VkPipeline CreateGraphicsPipeline(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx);
    VkPipeline CreateComputePipeline(MComputeDispatcher* pComputeDispatcher);

    void GenerateShaderParamSet(MShaderParamSet* pParamSet);
    void DestroyShaderParamSet(MShaderParamSet* pParamSet);
    void AllocateShaderParamSet(MShaderParamSet* pParamSet);

    void DestroyShaderParamSetImpl(MShaderParamSet* pParamSet);

private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;
    MRepeatIDPool<uint32_t> m_RenderPassIDPool;
    MRepeatIDPool<uint32_t> m_ComputeDispatcherIDPool;

    std::vector<MMaterialPipelineGroup*> m_tPipelineTable;

    std::vector<MMaterialPipelineLayoutData*> m_vPipelineLayouts;

	std::map<uint32_t, std::shared_ptr<MMaterial>> m_tMaterialMap;

    std::vector<MComputeDispatcherData*> m_tComputeDispatcherData;

    std::map<uint32_t, MComputeDispatcher*> m_tComputeDispatcherMap;

    MVulkanDevice* m_pDevice;

};


#endif

#endif