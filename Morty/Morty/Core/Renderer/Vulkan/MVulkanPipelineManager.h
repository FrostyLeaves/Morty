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
#include "MVulkanDevice.h"


struct MIndexPipeline
{
    std::vector<VkPipeline> m_vPipelines;
};

struct MRenderTargetPipeline
{
    std::vector<MIndexPipeline> vMaterials;
};

class MMaterial;
class MIRenderTarget;
class MORTY_CLASS MVulkanPipelineManager
{
public:
    MVulkanPipelineManager(MVulkanDevice* pDevice);
    virtual ~MVulkanPipelineManager();

public:

    VkPipeline FindPipeline(MMaterial* pMaterial, MIRenderTarget* pRenderTarget, const uint32_t& unIndex);

    VkPipelineLayout FindPipelineLayout(MMaterial* pMaterial);

public:
	void RegisterMaterial(MMaterial* pMaterial);
	void UnRegisterMaterial(MMaterial* pMaterial);


public:

    VkPipelineLayout CreateMaterialPipelineLayout(MMaterial* pMaterial);

    void DestroyPipelineLayout(VkPipelineLayout& pPepelineLayout);

private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;

    std::vector<MRenderTargetPipeline> m_vRenderTargets;

    std::vector<VkPipelineLayout> m_vPipelineLayouts;

    MVulkanDevice* m_pDevice;
};


#endif

#endif