#include "MVulkanPipelineManager.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MMaterial.h"
#include "MVulkanDevice.h"

MMaterialPipelineLayoutData::MMaterialPipelineLayoutData()
	: pipelineLayout(VK_NULL_HANDLE)
	, vSetLayouts()
	, vDescriptorSets()
{

}

MVulkanPipelineManager::MVulkanPipelineManager(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

MVulkanPipelineManager::~MVulkanPipelineManager()
{

}

void MVulkanPipelineManager::Release()
{
	for (MMaterialPipelineLayoutData& data : m_vPipelineLayouts)
		DestroyMaterialPipelineLayout(data);
	
	m_vPipelineLayouts.clear();


	for (MPipelineRenderPassGroup& group : m_vRenderPassGroup)
	{
		for (VkPipeline& pipeline : group.vMaterialGroup)
		{
			m_pDevice->m_BufferManager.DestroyPipelineLater(0, pipeline);
		}
	}

	m_vRenderPassGroup.clear();
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

MMaterialPipelineLayoutData* MVulkanPipelineManager::FindPipelineLayout(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();

	if (m_vPipelineLayouts.size() < id + 1)
		m_vPipelineLayouts.resize(id + 1);

	if (VK_NULL_HANDLE != m_vPipelineLayouts[id].pipelineLayout)
		return &m_vPipelineLayouts[id];

	CreateMaterialPipelineLayout(pMaterial, m_vPipelineLayouts[id]);

	return &m_vPipelineLayouts[id];
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
		if (m_vPipelineLayouts[id].pipelineLayout)
		{
			DestroyMaterialPipelineLayout(m_vPipelineLayouts[id]);
		}
	}

	for (MPipelineRenderPassGroup& group : m_vRenderPassGroup)
	{
		if (id < group.vMaterialGroup.size() && group.vMaterialGroup[id])
		{
			m_pDevice->m_BufferManager.DestroyPipelineLater(0, group.vMaterialGroup[id]);
			group.vMaterialGroup[id] = VK_NULL_HANDLE;
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
		for (VkPipeline& pipeline : group.vMaterialGroup)
		{
			m_pDevice->m_BufferManager.DestroyPipelineLater(0, pipeline);
		}
		group.vMaterialGroup.clear();
	}

	m_RenderPassIDPool.RecoveryID(id);
}

void MVulkanPipelineManager::BindDescriptor(MShaderParam& param)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = param.m_VkBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = param.var.GetSize();

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = param.m_VkDescriptorSet;
	descriptorWrite.dstBinding = param.unBinding;
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

	vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);
}

bool MVulkanPipelineManager::CreateMaterialPipelineLayout(MMaterial* pMaterial, MMaterialPipelineLayoutData& data)
{
	std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> tParamBinding;
	uint32_t unMaxSet = 0;

	for (MShaderParam& param : *pMaterial->GetShaderParams())
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = param.unBinding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = 0;
		if (param.eShaderType & MEShaderParamType::EVertex)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (param.eShaderType & MEShaderParamType::EPixel)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		tParamBinding[param.unSet].push_back(uboLayoutBinding);
		if (param.unSet > unMaxSet) unMaxSet = param.unSet;
	}

	for (MShaderTextureParam& param : *pMaterial->GetTextureParams())
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = param.unBinding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = 0;
		if (param.eShaderType & MEShaderParamType::EVertex)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (param.eShaderType & MEShaderParamType::EPixel)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		tParamBinding[param.unSet].push_back(uboLayoutBinding);
		if (param.unSet > unMaxSet) unMaxSet = param.unSet;
	}

	for (MShaderSampleParam& param : *pMaterial->GetSampleParams())
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = param.unBinding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = 0;
		if (param.eShaderType & MEShaderParamType::EVertex)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (param.eShaderType & MEShaderParamType::EPixel)
			uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;


		uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_BufferManager.m_VkDefaultSampler;

		tParamBinding[param.unSet].push_back(uboLayoutBinding);
		if (param.unSet > unMaxSet) unMaxSet = param.unSet;
	}

	std::vector<VkDescriptorSetLayout>& vSetLayouts = data.vSetLayouts;
	std::unordered_map<uint32_t, uint32_t> vParamLayoutsSet;

	for (uint32_t i = 0; i <= unMaxSet; ++i)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = tParamBinding[i].size();
		layoutInfo.pBindings = tParamBinding[i].data();

		vSetLayouts.push_back(VkDescriptorSetLayout());

		if (vkCreateDescriptorSetLayout(m_pDevice->m_VkDevice, &layoutInfo, nullptr, &vSetLayouts.back()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = vSetLayouts.size();
	pipelineLayoutInfo.pSetLayouts = vSetLayouts.data();

	VkPipelineLayout& pipelineLayout = data.pipelineLayout;
	if (vkCreatePipelineLayout(m_pDevice->m_VkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		return VK_NULL_HANDLE;


	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pDevice->m_BufferManager.m_VkDescriptorPool;
	allocInfo.descriptorSetCount = vSetLayouts.size();
	allocInfo.pSetLayouts = vSetLayouts.data();


	if (!vSetLayouts.empty())
	{
		std::vector<VkDescriptorSet>& vDescriptorSets = data.vDescriptorSets;
		vDescriptorSets.resize(vSetLayouts.size());
		if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, vDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (MShaderParam& param : *pMaterial->GetShaderParams())
		{
			param.m_VkDescriptorSet = vDescriptorSets[param.unSet];
			BindDescriptor(param);
		}

		for (MShaderTextureParam& param : *pMaterial->GetTextureParams())
			param.m_VkDescriptorSet = vDescriptorSets[param.unSet];

		for (MShaderSampleParam& param : *pMaterial->GetSampleParams())
		{
			param.m_VkDescriptorSet = vDescriptorSets[param.unSet];
		}
	}

	return pipelineLayout;
}

void MVulkanPipelineManager::DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData& data)
{
	if (data.pipelineLayout)
	{
		VkDevice& device = m_pDevice->m_VkDevice;

		m_pDevice->m_BufferManager.DestroyPipelineLayoutLater(0, data.pipelineLayout);

		if (!data.vDescriptorSets.empty())
			m_pDevice->m_BufferManager.DestroyDescriptorSets(0, data.vDescriptorSets);

		for (VkDescriptorSetLayout& layout : data.vSetLayouts)
			m_pDevice->m_BufferManager.DestroyDescriptorSetLayoutLater(0, layout);

		data.pipelineLayout = VK_NULL_HANDLE;
		data.vDescriptorSets.clear();
		data.vSetLayouts.clear();
	}
}



#endif

