#include "MVulkanPipelineManager.h"

#include "MMaterial.h"

MVulkanPipelineManager::MVulkanPipelineManager()
{

}

MVulkanPipelineManager::~MVulkanPipelineManager()
{

}

MPipeline* MVulkanPipelineManager::FindPipeline(MMaterial* pMaterial, MIRenderTarget* pRenderTarget, const uint32_t& unIndex)
{
	return nullptr;
}

uint32_t MVulkanPipelineManager::RegisterMaterial(MMaterial* pMaterial)
{
	return m_MaterialIDPool.GetNewID();
}

void MVulkanPipelineManager::UnRegisterMaterial(MMaterial* pMaterial)
{
	m_MaterialIDPool.RecoveryID(pMaterial->GetMaterialID());
}
