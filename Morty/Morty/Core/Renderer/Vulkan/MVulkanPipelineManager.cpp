#include "MVulkanPipelineManager.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MMaterial.h"
#include "MVulkanDevice.h"
#include "MFunction.h"

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
	for (MMaterialPipelineLayoutData* pLayoutData : m_vPipelineLayouts)
	{
		if (pLayoutData)
		{
			DestroyMaterialPipelineLayout(pLayoutData);
			delete pLayoutData;
		}
	}
	m_vPipelineLayouts.clear();


	for (MMaterialPipelineGroup* pMaterialGroup : m_tPipelineTable)
	{
		if (pMaterialGroup)
		{
			for (MRenderPassPipelines* pRenderPassGroup : pMaterialGroup->vRenderPassGroup)
			{
				if (pRenderPassGroup)
				{
					for (VkPipeline pipeline : pRenderPassGroup->vSubpassPipeline)
						m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pipeline);

					delete pRenderPassGroup;
				}
			}

			delete pMaterialGroup;
		}
	}

	m_tPipelineTable.clear();
}

VkPipeline MVulkanPipelineManager::FindPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& unSubpassIdx)
{
	uint32_t unMaterialID = pMaterial->GetMaterialID();
	uint32_t unRenderPassID = pRenderPass->GetRenderPassID();

	if (m_tPipelineTable.size() < unRenderPassID + 1)
		return VK_NULL_HANDLE;

	MMaterialPipelineGroup* pMaterialGroup = m_tPipelineTable[unRenderPassID];
	if (!pMaterialGroup)
		return VK_NULL_HANDLE;

	if (pMaterialGroup->vRenderPassGroup.size() < unRenderPassID + 1)
		return VK_NULL_HANDLE;

	MRenderPassPipelines* pPipelines = pMaterialGroup->vRenderPassGroup[unRenderPassID];
	if (!pPipelines)
		return VK_NULL_HANDLE;

	if (pPipelines->vSubpassPipeline.size() < unSubpassIdx + 1)
		return VK_NULL_HANDLE;

	return pPipelines->vSubpassPipeline[unSubpassIdx];
}

void MVulkanPipelineManager::SetPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& unSubpassIdx, VkPipeline pipeline)
{
	uint32_t unMaterialID = pMaterial->GetMaterialID();
	uint32_t unRenderPassID = pRenderPass->GetRenderPassID();


	if (m_tPipelineTable.size() < unMaterialID + 1)
		m_tPipelineTable.resize(unMaterialID + 1, nullptr);


	MMaterialPipelineGroup* pMaterialGroup = m_tPipelineTable[unMaterialID];

	if (nullptr == pMaterialGroup)
	{
		pMaterialGroup = new MMaterialPipelineGroup();
		m_tPipelineTable[unMaterialID] = pMaterialGroup;
	}

	if (pMaterialGroup->vRenderPassGroup.size() < unRenderPassID + 1)
		pMaterialGroup->vRenderPassGroup.resize(unRenderPassID + 1, nullptr);

	MRenderPassPipelines* pRenderPassGroup = pMaterialGroup->vRenderPassGroup[unRenderPassID];
	if (nullptr == pRenderPassGroup)
	{
		pRenderPassGroup = new MRenderPassPipelines();
		pMaterialGroup->vRenderPassGroup[unRenderPassID] = pRenderPassGroup;
	}

	if (pRenderPassGroup->vSubpassPipeline.size() < unSubpassIdx + 1)
		pRenderPassGroup->vSubpassPipeline.resize(unSubpassIdx + 1, VK_NULL_HANDLE);

	if(pRenderPassGroup->vSubpassPipeline[unSubpassIdx])
	{
		m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pRenderPassGroup->vSubpassPipeline[unSubpassIdx]);
		pRenderPassGroup->vSubpassPipeline[unSubpassIdx] = VK_NULL_HANDLE;
	}

	pRenderPassGroup->vSubpassPipeline[unSubpassIdx] = pipeline;
}

MMaterialPipelineLayoutData* MVulkanPipelineManager::FindOrCreatePipelineLayout(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();

	if (m_vPipelineLayouts.size() < id + 1)
		m_vPipelineLayouts.resize(id + 1);

	if (nullptr != m_vPipelineLayouts[id])
		return m_vPipelineLayouts[id];

	m_vPipelineLayouts[id] = CreateMaterialPipelineLayout(pMaterial);

	return m_vPipelineLayouts[id];
}

MMaterialPipelineLayoutData* MVulkanPipelineManager::FindPipelineLayout(const uint32_t& nMaterialIdx)
{
	if (M_INVALID_INDEX == nMaterialIdx)
		return nullptr;

	if (m_vPipelineLayouts.size() < nMaterialIdx + 1)
		return nullptr;

	return m_vPipelineLayouts[nMaterialIdx];
}

void MVulkanPipelineManager::RegisterMaterial(MMaterial* pMaterial)
{
	uint32_t id = m_MaterialIDPool.GetNewID();
	pMaterial->SetMaterialID(id);

	m_tMaterialMap[id] = pMaterial;
}

void MVulkanPipelineManager::UnRegisterMaterial(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();
	m_tMaterialMap.erase(id);

	if (id < m_vPipelineLayouts.size())
	{
		if (m_vPipelineLayouts[id])
		{
			DestroyMaterialPipelineLayout(m_vPipelineLayouts[id]);
			delete m_vPipelineLayouts[id];
			m_vPipelineLayouts[id] = nullptr;
		}
	}

	if (m_tPipelineTable.size() < id + 1)
		return;

	MMaterialPipelineGroup* pMaterialGroup = m_tPipelineTable[id];
	if (!pMaterialGroup)
		return;

	for (MRenderPassPipelines* pipelines : pMaterialGroup->vRenderPassGroup)
	{
		if (pipelines)
		{
			for (VkPipeline pipeline : pipelines->vSubpassPipeline)
			{
				m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pipeline);
			}

			delete pipelines;
		}
	}

	delete pMaterialGroup;
	m_tPipelineTable[id] = nullptr;

	m_MaterialIDPool.RecoveryID(id);
}

void MVulkanPipelineManager::RegisterRenderPass(MRenderPass* pRenderPass)
{
	pRenderPass->SetRenderPassID(m_RenderPassIDPool.GetNewID());
}

void MVulkanPipelineManager::UnRegisterRenderPass(MRenderPass* pRenderPass)
{
	uint32_t id = pRenderPass->GetRenderPassID();


	for (MMaterialPipelineGroup* pMaterialGroup : m_tPipelineTable)
	{
		if (pMaterialGroup && id < pMaterialGroup->vRenderPassGroup.size())
		{
			if (MRenderPassPipelines* pPipelines = pMaterialGroup->vRenderPassGroup[id])
			{
				for (VkPipeline& pipeline : pPipelines->vSubpassPipeline)
				{
					m_pDevice->m_ObjectDestructor.DestroyPipelineLater(pipeline);
				}

				delete pPipelines;
				pMaterialGroup->vRenderPassGroup[id] = nullptr;
			}
		}
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

//	memset(pParam->bDirty, 1, sizeof(bool) * M_BUFFER_NUM);
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

		if (VK_NULL_HANDLE == imageInfo.sampler)
			imageInfo.sampler = m_pDevice->m_ObjectDestructor.m_VkDefaultSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = pParamSet->m_VkDescriptorSet[unIndex];
		descriptorWrite.dstBinding = pParam->unBinding;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imageInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		//A VkDescripotrSet can only be updated once on per render. .
		vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

MMaterialPipelineLayoutData* MVulkanPipelineManager::CreateMaterialPipelineLayout(MMaterial* pMaterial)
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

		for (MShaderSampleParam* param : paramSets.m_vSamples)
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

			if (param->unBinding == 4)
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_ObjectDestructor.m_VkLessEqualSampler;
			else
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_ObjectDestructor.m_VkDefaultSampler;

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}
	}

	std::vector<VkDescriptorSetLayout> vSetLayouts;
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

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	if (vkCreatePipelineLayout(m_pDevice->m_VkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		return VK_NULL_HANDLE;


	MMaterialPipelineLayoutData* pResult = new MMaterialPipelineLayoutData();

	pResult->pipelineLayout = pipelineLayout;
	pResult->pMaterial = pMaterial;
	pResult->vSetLayouts = vSetLayouts;

	return pResult;
}

void MVulkanPipelineManager::DestroyMaterialPipelineLayout(MMaterialPipelineLayoutData* pLayoutData)
{
	if (!pLayoutData)
		return;

	if (pLayoutData->pipelineLayout)
	{
		VkDevice& device = m_pDevice->m_VkDevice;

		m_pDevice->m_ObjectDestructor.DestroyPipelineLayoutLater(pLayoutData->pipelineLayout);

		for (VkDescriptorSetLayout& layout : pLayoutData->vSetLayouts)
			m_pDevice->m_ObjectDestructor.DestroyDescriptorSetLayoutLater(layout);

		pLayoutData->pipelineLayout = VK_NULL_HANDLE;
	}

	for (MShaderParamSet* pParamSet : pLayoutData->vShaderParamSets)
	{
		DestroyShaderParamSetImpl(pParamSet);
	}
	pLayoutData->vShaderParamSets.clear();

	pLayoutData->vSetLayouts.clear();
	pLayoutData->pMaterial = nullptr;
}

void MVulkanPipelineManager::GenerateShaderParamSet(MShaderParamSet* pParamSet)
{
	MMaterialPipelineLayoutData* pLayoutData = FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
		return;

	if (pParamSet->m_unKey >= pLayoutData->vSetLayouts.size())
		return;

	pLayoutData->vShaderParamSets.push_back(pParamSet);
	pParamSet->m_unLayoutDataIdx = pLayoutData->vShaderParamSets.size();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		VkDescriptorSetLayout vkDescriptorSetLayout = pLayoutData->vSetLayouts[pParamSet->m_unKey];

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_pDevice->m_ObjectDestructor.m_VkDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkDescriptorSetLayout;


		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
		{
			MLogManager::GetInstance()->Error("MVulkanPipelineManager::CreateMaterialDescriptorSet error: descriptor pool == 0");
			return;
		}

		pParamSet->m_VkDescriptorSet[i] = descriptorSet;
	}

	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		m_pDevice->GenerateShaderParamBuffer(pParam);

		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
		{
			BindConstantParam(pParamSet, pParam, i);
		}
	}
}

void MVulkanPipelineManager::DestroyShaderParamSet(MShaderParamSet* pParamSet)
{
	MMaterialPipelineLayoutData* pLayoutData = FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
		return;

	if (M_INVALID_INDEX != pParamSet->m_unLayoutDataIdx)
	{
		for (uint32_t i = pParamSet->m_unLayoutDataIdx; i < pLayoutData->vShaderParamSets.size() - 1; ++i)
		{
			pLayoutData->vShaderParamSets[i] = pLayoutData->vShaderParamSets[i + 1];
		}

		pLayoutData->vShaderParamSets.pop_back();
	}

	DestroyShaderParamSetImpl(pParamSet);
}

void MVulkanPipelineManager::DestroyShaderParamSetImpl(MShaderParamSet* pParamSet)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (pParamSet->m_VkDescriptorSet[i])
		{
			m_pDevice->m_ObjectDestructor.DestroyDescriptorSetLater(pParamSet->m_VkDescriptorSet[i]);
		}
	}

	memset(pParamSet->m_VkDescriptorSet, VK_NULL_HANDLE, sizeof(VkDescriptorSet) * M_BUFFER_NUM);

	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		m_pDevice->DestroyShaderParamBuffer(pParam);
		pParam->SetDirty();
	}

	for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
	{
		pParam->SetDirty();
	}


	pParamSet->m_unLayoutDataIdx = M_INVALID_INDEX;
	pParamSet->m_nDescriptorSetInitMaterialIdx = M_INVALID_INDEX;
}

#endif
