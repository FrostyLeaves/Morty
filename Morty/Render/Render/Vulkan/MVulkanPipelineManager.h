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
struct MShaderTextureParam;
struct MShaderConstantParam;


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

class MORTY_API MVulkanPipelineManager
{
public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);
    virtual ~MVulkanPipelineManager();

    void Release();

public:

    VkPipeline FindPipeline(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, const uint32_t& unSubpassIdx);

    void SetPipeline(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, const uint32_t& unSubpassIdx, VkPipeline pipeline);

	MMaterialPipelineLayoutData* FindOrCreatePipelineLayout(std::shared_ptr<MMaterial> pMaterial);
	MMaterialPipelineLayoutData* FindPipelineLayout(const uint32_t& nMaterialIdx);

public:
	bool RegisterMaterial(std::shared_ptr<MMaterial> pMaterial);
	bool UnRegisterMaterial(std::shared_ptr<MMaterial> pMaterial);

    void RegisterRenderPass(MRenderPass* pRenderPass);
    void UnRegisterRenderPass(MRenderPass* pRenderPass);

public:

	void BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam);

    void BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam);

    MMaterialPipelineLayoutData* CreateMaterialPipelineLayout(std::shared_ptr<MMaterial> pMaterial);
    void DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData* pLayoutData);

    void GenerateShaderParamSet(MShaderParamSet* pParamSet);
    void DestroyShaderParamSet(MShaderParamSet* pParamSet);
    void AllocateShaderParamSet(MShaderParamSet* pParamSet);

    void DestroyShaderParamSetImpl(MShaderParamSet* pParamSet);

private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;
    MRepeatIDPool<uint32_t> m_RenderPassIDPool;

    std::vector<MMaterialPipelineGroup*> m_tPipelineTable;

    std::vector<MMaterialPipelineLayoutData*> m_vPipelineLayouts;

	std::map<uint32_t, std::shared_ptr<MMaterial>> m_tMaterialMap;

    MVulkanDevice* m_pDevice;

};


#endif

#endif