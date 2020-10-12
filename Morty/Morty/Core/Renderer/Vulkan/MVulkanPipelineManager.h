/**
 * @File         MVulkanPipelineManager
 * 
 * @Created      2020-06-23 12:16:06
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVULKANPIPELINEMANAGER_H_
#define _M_MVULKANPIPELINEMANAGER_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MIDPool.h"
#include "MRenderPass.h"

class MVulkanDevice;
class MShaderParamSet;
class MShaderTextureParam;
class MShaderConstantParam;
struct MPipelineRenderPassGroup
{
    std::vector<VkPipeline> vMaterialGroup;
};

struct MMaterialPipelineLayoutData
{
    MMaterialPipelineLayoutData();

    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> vSetLayouts;
};

class MMaterial;
class MIRenderTarget;
class MORTY_CLASS MVulkanPipelineManager
{
public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);
    virtual ~MVulkanPipelineManager();

    void Release();

public:

    VkPipeline FindPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass);

    void SetPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipeline pipeline);

	MMaterialPipelineLayoutData* FindOrCreatePipelineLayout(MMaterial* pMaterial);
	MMaterialPipelineLayoutData* FindPipelineLayout(const uint32_t& nMaterialIdx);

public:
	void RegisterMaterial(MMaterial* pMaterial);
	void UnRegisterMaterial(MMaterial* pMaterial);

    void RegisterRenderPass(MRenderPass* pRenderPass);
    void UnRegisterRenderPass(MRenderPass* pRenderPass);

public:

	void BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam, const uint32_t& unIndex);

    void BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam, const uint32_t& unIndex);

    bool CreateMaterialPipelineLayout(MMaterial* pMaterial, MMaterialPipelineLayoutData& data);
    void DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData& data);

    VkDescriptorSet CreateMaterialDescriptorSet(VkDescriptorSetLayout& vkDescriptorSetLayout);


private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;
    MRepeatIDPool<uint32_t> m_RenderPassIDPool;

    std::vector<MPipelineRenderPassGroup> m_vRenderPassGroup;

    std::vector<MMaterialPipelineLayoutData> m_vPipelineLayouts;

    MVulkanDevice* m_pDevice;

};


#endif

#endif