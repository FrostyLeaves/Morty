#include "MVulkanPipelineManager.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MEngine.h"
#include "MMaterial.h"
#include "MFunction.h"
#include "MVulkanDevice.h"

MMaterialPipelineLayoutData::MMaterialPipelineLayoutData()
	: pMaterial(nullptr)
	, pipelineLayout(VK_NULL_HANDLE)
	, vSetLayouts()
	, vShaderParamSets()
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
						m_pDevice->GetRecycleBin()->DestroyPipelineLater(pipeline);

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

	if (m_tPipelineTable.size() < unMaterialID + 1)
		return VK_NULL_HANDLE;

	MMaterialPipelineGroup* pMaterialGroup = m_tPipelineTable[unMaterialID];
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
		m_pDevice->GetRecycleBin()->DestroyPipelineLater(pRenderPassGroup->vSubpassPipeline[unSubpassIdx]);
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
	if (MGlobal::M_INVALID_INDEX == nMaterialIdx)
		return nullptr;

	if (m_vPipelineLayouts.size() < nMaterialIdx + 1)
		return nullptr;

	return m_vPipelineLayouts[nMaterialIdx];
}

bool MVulkanPipelineManager::RegisterMaterial(MMaterial* pMaterial)
{
	uint32_t id = m_MaterialIDPool.GetNewID();
	pMaterial->SetMaterialID(id);

	m_tMaterialMap[id] = pMaterial;

	return true;
}

bool MVulkanPipelineManager::UnRegisterMaterial(MMaterial* pMaterial)
{
	uint32_t id = pMaterial->GetMaterialID();
	if (m_tMaterialMap.find(id) == m_tMaterialMap.end())
		return false;

	m_tMaterialMap.erase(id);
	m_MaterialIDPool.RecoveryID(id);

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
		return true;

	MMaterialPipelineGroup* pMaterialGroup = m_tPipelineTable[id];
	if (!pMaterialGroup)
		return true;

	for (MRenderPassPipelines* pipelines : pMaterialGroup->vRenderPassGroup)
	{
		if (pipelines)
		{
			for (VkPipeline pipeline : pipelines->vSubpassPipeline)
			{
				m_pDevice->GetRecycleBin()->DestroyPipelineLater(pipeline);
			}

			delete pipelines;
		}
	}

	delete pMaterialGroup;
	m_tPipelineTable[id] = nullptr;

	return true;
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
					m_pDevice->GetRecycleBin()->DestroyPipelineLater(pipeline);
				}

				delete pPipelines;
				pMaterialGroup->vRenderPassGroup[id] = nullptr;
			}
		}
	}
	
	if (id != MGlobal::M_INVALID_INDEX)
	{
		m_RenderPassIDPool.RecoveryID(id);
	}
	pRenderPass->SetRenderPassID(MGlobal::M_INVALID_INDEX);
}

void MVulkanPipelineManager::BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = pParam->m_VkBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = pParam->var.GetSize();

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = pParamSet->m_VkDescriptorSet;
	descriptorWrite.dstBinding = pParam->unBinding;
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

	vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);
}

void MVulkanPipelineManager::BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam)
{
	MTexture* pTexture = pParam->pTexture;
	if (!pTexture) pTexture = &m_pDevice->m_ShaderDefaultTexture;

	if (pTexture)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageView = pTexture->m_VkImageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = pTexture->m_VkSampler;

		if (VK_NULL_HANDLE == imageInfo.sampler)
			imageInfo.sampler = m_pDevice->m_VkLinearSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = pParamSet->m_VkDescriptorSet;
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
	std::vector<VkDescriptorSetLayoutBinding> vParamBinding[MRenderGlobal::SHADER_PARAM_SET_NUM];
	
	for (uint32_t unSetIdx = 0; unSetIdx < MRenderGlobal::SHADER_PARAM_SET_NUM; ++unSetIdx)
	{
		MShaderParamSet& paramSets = pMaterial->GetShaderParamSets()[unSetIdx];

		for (MShaderConstantParam* param : paramSets.m_vParams)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

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
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (MShaderSampleParam* param : paramSets.m_vSamples)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			if(param->eSamplerType == MESamplerType::ELinear)
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_VkLinearSampler;
			else
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_VkNearestSampler;

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}
	}

	std::vector<VkDescriptorSetLayout> vSetLayouts;
	std::unordered_map<uint32_t, uint32_t> vParamLayoutsSet;

	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		
		layoutInfo.bindingCount = vParamBinding[i].size();
		layoutInfo.pBindings = vParamBinding[i].data();

//		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

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
		m_pDevice->GetRecycleBin()->DestroyPipelineLayoutLater(pLayoutData->pipelineLayout);

		for (VkDescriptorSetLayout& layout : pLayoutData->vSetLayouts)
			m_pDevice->GetRecycleBin()->DestroyDescriptorSetLayoutLater(layout);

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

	pParamSet->m_unLayoutDataIdx = pLayoutData->vShaderParamSets.size();
	pLayoutData->vShaderParamSets.push_back(pParamSet);

	VkDescriptorSetLayout vkDescriptorSetLayout = pLayoutData->vSetLayouts[pParamSet->m_unKey];

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pDevice->m_VkDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkDescriptorSetLayout;

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		m_pDevice->GetEngine()->GetLogger()->Error("MVulkanPipelineManager::CreateMaterialDescriptorSet error: descriptor pool == 0");
		return;
	}

	pParamSet->m_VkDescriptorSet = descriptorSet;
}

void MVulkanPipelineManager::AllocateShaderParamSet(MShaderParamSet* pParamSet)
{
	MMaterialPipelineLayoutData* pLayoutData = FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
		return;

	if (pParamSet->m_VkDescriptorSet)
	{
		m_pDevice->GetRecycleBin()->DestroyDescriptorSetLater(pParamSet->m_VkDescriptorSet);
		pParamSet->m_VkDescriptorSet = VK_NULL_HANDLE;
	}

	VkDescriptorSetLayout vkDescriptorSetLayout = pLayoutData->vSetLayouts[pParamSet->m_unKey];

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_pDevice->m_VkDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &vkDescriptorSetLayout;

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(m_pDevice->m_VkDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
	{
		m_pDevice->GetEngine()->GetLogger()->Error("MVulkanPipelineManager::AllocateShaderParamSet error: descriptor pool == 0");
		return;
	}

	pParamSet->m_VkDescriptorSet = descriptorSet;
}

void MVulkanPipelineManager::DestroyShaderParamSet(MShaderParamSet* pParamSet)
{
	if (MMaterialPipelineLayoutData* pLayoutData = FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx))
	{
		if (MGlobal::M_INVALID_INDEX != pParamSet->m_unLayoutDataIdx)
		{
			if (pParamSet->m_unLayoutDataIdx < pLayoutData->vShaderParamSets.size())
			{
				for (uint32_t i = pParamSet->m_unLayoutDataIdx; i < pLayoutData->vShaderParamSets.size() - 1; ++i)
				{
					pLayoutData->vShaderParamSets[i] = pLayoutData->vShaderParamSets[i + 1];
					pLayoutData->vShaderParamSets[i]->m_unLayoutDataIdx = i;
				}

				pLayoutData->vShaderParamSets.pop_back();
			}
		}

		pParamSet->m_unLayoutDataIdx = MGlobal::M_INVALID_INDEX;
		pParamSet->m_nDescriptorSetInitMaterialIdx = MGlobal::M_INVALID_INDEX;
	}

	DestroyShaderParamSetImpl(pParamSet);
}

void MVulkanPipelineManager::DestroyShaderParamSetImpl(MShaderParamSet* pParamSet)
{
	if (!pParamSet)
		return;

	if (pParamSet->m_VkDescriptorSet)
	{
		m_pDevice->GetRecycleBin()->DestroyDescriptorSetLater(pParamSet->m_VkDescriptorSet);
		pParamSet->m_VkDescriptorSet = VK_NULL_HANDLE;
	}
}

#endif
