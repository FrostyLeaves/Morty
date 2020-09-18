#include "MVulkanPipelineManager.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MMaterial.h"
#include "MVulkanDevice.h"

MMaterialPipelineLayoutData::MMaterialPipelineLayoutData()
	: pipelineLayout(VK_NULL_HANDLE)
	, vSetLayouts()
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
			m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pipeline);
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
			m_pDevice->m_ObjectDestructor.DestroyPipelineLater(group.vMaterialGroup[id]);
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
			m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pipeline);
		}
		group.vMaterialGroup.clear();
	}

	m_RenderPassIDPool.RecoveryID(id);
}

void MVulkanPipelineManager::BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam, const uint32_t& unIndex)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = pParam->m_VkBuffer[unIndex];
	bufferInfo.offset = 0;
	bufferInfo.range = pParam->var.GetSize();

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = pParamSet->m_VkDescriptorSet[unIndex];
	descriptorWrite.dstBinding = pParam->unBinding;
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

	vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);
}

void MVulkanPipelineManager::BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam, const uint32_t& unIndex)
{
	MITexture* pTexture = pParam->pTexture;
	if (!pTexture) pTexture = &m_pDevice->m_WhiteTexture;

	if (MTextureBuffer* pBuffer = pTexture->GetBuffer())
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageView = pBuffer->m_VkImageView;
		imageInfo.imageLayout = pBuffer->m_VkImageLayout;
		imageInfo.sampler = pBuffer->m_VkSampler;

		if (VK_NULL_HANDLE == imageInfo.sampler)		//TODO Default Sampler
			imageInfo.sampler = m_pDevice->m_ObjectDestructor.m_VkDefaultSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = pParamSet->m_VkDescriptorSet[unIndex];
		descriptorWrite.dstBinding = pParam->unBinding;
		descriptorWrite.dstArrayElement = 0;

		//descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imageInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

bool MVulkanPipelineManager::CreateMaterialPipelineLayout(MMaterial* pMaterial, MMaterialPipelineLayoutData& data)
{
	std::vector<VkDescriptorSetLayoutBinding> vParamBinding[M_VALID_SHADER_SET_NUM];
	
	for (uint32_t unSetIdx = 0; unSetIdx < M_VALID_SHADER_SET_NUM; ++unSetIdx)
	{
		MShaderParamSet& paramSets = pMaterial->GetShaderParamSets()[unSetIdx];

		for (MShaderConstantParam* param : paramSets.m_vParams)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = 0;
			if (param->eShaderType & MEShaderParamType::EVertex)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
			if (param->eShaderType & MEShaderParamType::EPixel)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (MShaderTextureParam* param : paramSets.m_vTextures)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			//uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = 0;
			if (param->eShaderType & MEShaderParamType::EVertex)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
			if (param->eShaderType & MEShaderParamType::EPixel)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (MShaderSampleParam* param : paramSets.m_vSamples)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = 0;
			if (param->eShaderType & MEShaderParamType::EVertex)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
			if (param->eShaderType & MEShaderParamType::EPixel)
				uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

			if (param->unBinding == 4)
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_ObjectDestructor.m_VkLessEqualSampler;
			else
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_ObjectDestructor.m_VkDefaultSampler;

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}
	}

	std::vector<VkDescriptorSetLayout>& vSetLayouts = data.vSetLayouts;
	std::unordered_map<uint32_t, uint32_t> vParamLayoutsSet;

	for (uint32_t i = 0; i < M_VALID_SHADER_SET_NUM; ++i)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = vParamBinding[i].size();
		layoutInfo.pBindings = vParamBinding[i].data();

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


	return pipelineLayout;
}

void MVulkanPipelineManager::DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData& data)
{
	if (data.pipelineLayout)
	{
		VkDevice& device = m_pDevice->m_VkDevice;

		m_pDevice->m_ObjectDestructor.DestroyPipelineLayoutLater(data.pipelineLayout);

		for (VkDescriptorSetLayout& layout : data.vSetLayouts)
			m_pDevice->m_ObjectDestructor.DestroyDescriptorSetLayoutLater(layout);

		data.pipelineLayout = VK_NULL_HANDLE;
		data.vSetLayouts.clear();
	}
}

VkDescriptorSet MVulkanPipelineManager::CreateMaterialDescriptorSet(MMaterialPipelineLayoutData& data, const uint32_t& unSetIdx)
{
	VkDescriptorSet descriptorSet;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pDevice->m_ObjectDestructor.m_VkDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &data.vSetLayouts[unSetIdx];

	if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return descriptorSet;
}

#endif
