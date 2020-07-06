#include "MVulkanPipelineManager.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MMaterial.h"
#include "MVulkanDevice.h"

MVulkanPipelineManager::MVulkanPipelineManager(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

MVulkanPipelineManager::~MVulkanPipelineManager()
{

}

VkPipeline MVulkanPipelineManager::FindPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass)
{
	uint32_t unRenderPassID = pRenderPass->GetRenderPassID();
	if (m_vRenderPassGroup.size() < unRenderPassID + 1)
		return VK_NULL_HANDLE;

	MPipelineRenderPassGroup& group = m_vRenderPassGroup[unRenderPassID];
	uint32_t unMaterialID = pMaterial->GetMaterialID();

	if (group.vMaterialGroup.size() < unMaterialID + 1)
		return VK_NULL_HANDLE;

	return group.vMaterialGroup[unMaterialID];
}

void MVulkanPipelineManager::SetPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipeline pipeline)
{
	uint32_t unRenderPassID = pRenderPass->GetRenderPassID();
	if (m_vRenderPassGroup.size() < unRenderPassID + 1)
		m_vRenderPassGroup.resize(unRenderPassID + 1);


	MPipelineRenderPassGroup& group = m_vRenderPassGroup[unRenderPassID];
	uint32_t unMaterialID = pMaterial->GetMaterialID();

	if (group.vMaterialGroup.size() < unMaterialID + 1)
		group.vMaterialGroup.resize(unMaterialID + 1, VK_NULL_HANDLE);

	if (group.vMaterialGroup[unMaterialID] != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_pDevice->m_VkDevice, group.vMaterialGroup[unMaterialID], nullptr);
		group.vMaterialGroup[unMaterialID] = VK_NULL_HANDLE;
	}

	group.vMaterialGroup[unMaterialID] = pipeline;
}

VkPipelineLayout MVulkanPipelineManager::FindPipelineLayout(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();

	if (m_vPipelineLayouts.size() < id + 1)
		m_vPipelineLayouts.resize(id + 1, VK_NULL_HANDLE);

	if (VK_NULL_HANDLE != m_vPipelineLayouts[id])
		return m_vPipelineLayouts[id];

	m_vPipelineLayouts[id] = CreateMaterialPipelineLayout(pMaterial);

	return m_vPipelineLayouts[id];
}

void MVulkanPipelineManager::RegisterMaterial(MMaterial* pMaterial)
{
	pMaterial->SetMaterialID(m_MaterialIDPool.GetNewID());
}

void MVulkanPipelineManager::UnRegisterMaterial(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();

	if (id < m_vPipelineLayouts.size())
	{
		if (m_vPipelineLayouts[id])
		{
			DestroyPipelineLayout(m_vPipelineLayouts[id]);
			m_vPipelineLayouts[id] = VK_NULL_HANDLE;
		}
	}

	m_MaterialIDPool.RecoveryID(id);
}

void MVulkanPipelineManager::RegisterRenderPass(MRenderPass* pRenderPass)
{
	pRenderPass->SetRenderPassID(m_RenderPassIDPool.GetNewID());
}

void MVulkanPipelineManager::UnRegisterRenderPass(MRenderPass* pRenderPass)
{
	uint32_t id = pRenderPass->GetRenderPassID();

	if (id < m_vRenderPassGroup.size())
	{
		MPipelineRenderPassGroup& group = m_vRenderPassGroup[id];
		group.vMaterialGroup.clear();
	}

	m_RenderPassIDPool.RecoveryID(id);
}

VkPipelineLayout MVulkanPipelineManager::CreateMaterialPipelineLayout(MMaterial* pMaterial)
{
// 	if (pMaterial->GetShaderParams()->empty())
// 		return VK_NULL_HANDLE;
	
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> tSetLayoutBinding;

	for (const MShaderParam& param : *pMaterial->GetShaderParams())
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = param.unBinding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = 0;
		if (param.eType & MEShaderParamType::EVertex)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (param.eType & MEShaderParamType::EPixel)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		tSetLayoutBinding[param.unSet].push_back(uboLayoutBinding);
	}

	std::vector<VkDescriptorSetLayout> vSetLayouts;
	std::vector<uint32_t> vSetLayoutsSet;

	for (const auto& pair : tSetLayoutBinding)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = pair.second.size();
		layoutInfo.pBindings = pair.second.data();

		vSetLayouts.push_back(VkDescriptorSetLayout());
		vSetLayoutsSet.push_back(pair.first);

		if (vkCreateDescriptorSetLayout(m_pDevice->m_VkDevice, &layoutInfo, nullptr, &vSetLayouts.back()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = vSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = vSetLayouts.data();

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(m_pDevice->m_VkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		return VK_NULL_HANDLE;


	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pDevice->m_VkDescriptorPool;
	allocInfo.descriptorSetCount = vSetLayouts.size();
	allocInfo.pSetLayouts = vSetLayouts.data();


	if (!vSetLayouts.empty())
	{
		std::vector<VkDescriptorSet> descriptorSets;
		descriptorSets.resize(vSetLayouts.size());
		if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (uint32_t setIdx = 0; setIdx < vSetLayoutsSet.size(); ++setIdx)
		{
			for (MShaderParam& param : *pMaterial->GetShaderParams())
			{
				if (param.unSet == vSetLayoutsSet[setIdx])
				{
					param.m_VkDescriptorSet = descriptorSets[setIdx];
				}
			}
		}
	}

	return pipelineLayout;
}

void MVulkanPipelineManager::DestroyPipelineLayout(VkPipelineLayout& pPepelineLayout)
{
	vkDestroyPipelineLayout(m_pDevice->m_VkDevice, pPepelineLayout, nullptr);
}



#endif