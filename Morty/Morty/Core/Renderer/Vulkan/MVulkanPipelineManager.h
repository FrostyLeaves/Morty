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

#include "MIDPool.h"

struct MPipeline
{

};

struct MIndexPipeline
{
    std::vector< MPipeline> m_vPipelines;
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
    MVulkanPipelineManager();
    virtual ~MVulkanPipelineManager();

public:

    MPipeline* FindPipeline(MMaterial* pMaterial, MIRenderTarget* pRenderTarget, const uint32_t& unIndex);

public:
	virtual uint32_t RegisterMaterial(MMaterial* pMaterial) ;
	virtual void UnRegisterMaterial(MMaterial* pMaterial) ;
private:

	MRepeatIDPool<uint32_t> m_MaterialIDPool;

    std::vector<MRenderTargetPipeline> m_vRenderTargets;
};


#endif
