﻿#include "Render/Vulkan/MVulkanPipelineManager.h"

#include "Render/MBuffer.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "Render/Vulkan/MVulkanDevice.h"

#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"


MVulkanPipelineManager::MVulkanPipelineManager(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

MVulkanPipelineManager::~MVulkanPipelineManager()
{

}

void MVulkanPipelineManager::Release()
{
	for (auto& pr: m_tPipelineTable)
	{
		if (pr.second)
		{
			DestroyPipeline(pr.second);
		}
	}

	m_tPipelineTable.clear();
}

std::shared_ptr<MGraphicsPipeline> MVulkanPipelineManager::FindOrCreateGraphicsPipeline(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass)
{
	MPipelineKey key(pMaterial->GetShaderProgram(), pRenderPass);

	auto&& findResult = m_tPipelineTable.find(key);
	if (findResult != m_tPipelineTable.end())
	{
		return std::dynamic_pointer_cast<MGraphicsPipeline>(findResult->second);
	}

	const std::shared_ptr<MShaderProgram>& pShaderProgram = pMaterial->GetShaderProgram();

	if (!pShaderProgram)
	{
		MORTY_ASSERT(pShaderProgram);
		return nullptr;
	}

	std::shared_ptr<MGraphicsPipeline> pPipeline = std::make_shared<MGraphicsPipeline>();
	m_tPipelineTable[key] = pPipeline;

	if (pRenderPass->m_vSubpass.empty())
	{
		pPipeline->m_vSubpassPipeline.push_back(CreateGraphicsPipeline(pPipeline, pMaterial, pRenderPass, 0));
		return pPipeline;
	}

	for (size_t nSubPassIdx = 0; nSubPassIdx < pRenderPass->m_vSubpass.size(); ++nSubPassIdx)
	{
		pPipeline->m_vSubpassPipeline.push_back(CreateGraphicsPipeline(pPipeline, pMaterial, pRenderPass, nSubPassIdx));
	}

	return pPipeline;
}

std::shared_ptr<MComputePipeline> MVulkanPipelineManager::FindOrCreateComputePipeline(MComputeDispatcher* pComputeDispatcher)
{
	MPipelineKey key(pComputeDispatcher->GetShaderProgram(), nullptr);

	auto&& findResult = m_tPipelineTable.find(key);
	if (findResult != m_tPipelineTable.end())
	{
		return std::dynamic_pointer_cast<MComputePipeline>(findResult->second);
	}

	const std::shared_ptr<MShaderProgram>& pShaderProgram = pComputeDispatcher->GetShaderProgram();

	if (!pShaderProgram)
	{
		MORTY_ASSERT(pShaderProgram);
		return nullptr;
	}

	std::shared_ptr<MComputePipeline> pPipeline = std::make_shared<MComputePipeline>();
	m_tPipelineTable[key] = pPipeline;

	pPipeline->m_vkPipeline = CreateComputePipeline(pPipeline, pComputeDispatcher);
	
	return pPipeline;
}

bool MVulkanPipelineManager::RegisterMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	return true;
}

bool MVulkanPipelineManager::UnRegisterMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	return true;
}

void MVulkanPipelineManager::RegisterRenderPass(MRenderPass* pRenderPass)
{
	pRenderPass->SetRenderPassID(m_RenderPassIDPool.GetNewID());
}

bool MVulkanPipelineManager::RegisterComputeDispatcher(MComputeDispatcher* pDispatcher)
{
	return true;
}

bool MVulkanPipelineManager::UnRegisterComputeDispatcher(MComputeDispatcher* pDispatcher)
{
	return true;
}

void MVulkanPipelineManager::UnRegisterRenderPass(MRenderPass* pRenderPass)
{
	for (auto iter = m_tPipelineTable.begin(); iter != m_tPipelineTable.end(); )
	{
		if (iter->first == pRenderPass)
		{
			DestroyPipeline(iter->second);
			iter = m_tPipelineTable.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	uint32_t id = pRenderPass->GetRenderPassID();

	if (id != MGlobal::M_INVALID_INDEX)
	{
		m_RenderPassIDPool.RecoveryID(id);
	}
	pRenderPass->SetRenderPassID(MGlobal::M_INVALID_INDEX);
}

void MVulkanPipelineManager::DestroyPipeline(const std::shared_ptr<MPipeline>& pipeline)
{
	if (!pipeline)
	{
		return;
	}

	if (std::shared_ptr<MGraphicsPipeline> graphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pipeline))
	{
		DestroyGraphicsPipeline(graphicsPipeline);
	}
	if (std::shared_ptr<MComputePipeline> graphicsPipeline = std::dynamic_pointer_cast<MComputePipeline>(pipeline))
	{
		DestroyComputePipeline(graphicsPipeline);
	}

	for (const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock : pipeline->m_tShaderPropertyBlocks)
	{
		DestroyShaderParamSetImpl(pPropertyBlock);
	}

	DestroyPipelineLayout(pipeline);

	pipeline->m_tShaderPropertyBlocks.clear();
}

void MVulkanPipelineManager::GeneratePipelineLayout(const std::shared_ptr<MPipeline>& pPipeline, const std::shared_ptr<MShaderProgram>& pShaderProgram)
{
	VkShaderStageFlags vkShaderStageFlags = pShaderProgram->GetUsage() == MShaderProgram::EUsage::ECompute ? VK_SHADER_STAGE_COMPUTE_BIT : VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayout> vSetLayouts;
	std::vector<VkDescriptorSetLayoutBinding> vParamBinding[MRenderGlobal::SHADER_PARAM_SET_NUM];

	for (uint32_t unSetIdx = 0; unSetIdx < MRenderGlobal::SHADER_PARAM_SET_NUM; ++unSetIdx)
	{
		std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = pShaderProgram->GetShaderParamSets()[unSetIdx];

		for (const std::shared_ptr<MShaderConstantParam>& param : pPropertyBlock->m_vParams)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = vkShaderStageFlags;

			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (const std::shared_ptr<MShaderTextureParam>& param : pPropertyBlock->m_vTextures)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			//uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = vkShaderStageFlags;

			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (const std::shared_ptr<MShaderSampleParam>& param : pPropertyBlock->m_vSamples)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = vkShaderStageFlags;

			if (param->eSamplerType == MESamplerType::ELinear)
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_VkLinearSampler;
			else
				uboLayoutBinding.pImmutableSamplers = &m_pDevice->m_VkNearestSampler;

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		for (const std::shared_ptr<MShaderStorageParam>& param : pPropertyBlock->m_vStorages)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = param->unBinding;
			uboLayoutBinding.descriptorType = param->m_VkDescriptorType;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = vkShaderStageFlags;

			vParamBinding[unSetIdx].push_back(uboLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		layoutInfo.bindingCount = vParamBinding[unSetIdx].size();
		layoutInfo.pBindings = vParamBinding[unSetIdx].data();

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

	MORTY_ASSERT(vkCreatePipelineLayout(m_pDevice->m_VkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

	pPipeline->m_pipelineLayout.vkPipelineLayout = pipelineLayout;
	pPipeline->m_pipelineLayout.vDescriptorSetLayouts = std::move(vSetLayouts);
}

void MVulkanPipelineManager::DestroyPipelineLayout(const std::shared_ptr<MPipeline>& pPipeline)
{
	m_pDevice->GetRecycleBin()->DestroyPipelineLayoutLater(pPipeline->m_pipelineLayout.vkPipelineLayout);

	for (VkDescriptorSetLayout& layout : pPipeline->m_pipelineLayout.vDescriptorSetLayouts)
	{
		m_pDevice->GetRecycleBin()->DestroyDescriptorSetLayoutLater(layout);
	}

	pPipeline->m_pipelineLayout.vkPipelineLayout = VK_NULL_HANDLE;
	pPipeline->m_pipelineLayout.vDescriptorSetLayouts = {};
}

void MVulkanPipelineManager::DestroyGraphicsPipeline(const std::shared_ptr<MGraphicsPipeline>& pGraphicsPipeline)
{
	if (!pGraphicsPipeline)
	{
		return;
	}

	for (auto& pipeline : pGraphicsPipeline->m_vSubpassPipeline)
	{
		m_pDevice->GetRecycleBin()->DestroyPipelineLater(pipeline);
		pipeline = nullptr;
	}
}

void MVulkanPipelineManager::DestroyComputePipeline(const std::shared_ptr<MComputePipeline>& pComputePipeline)
{
	if (!pComputePipeline)
		return;

	m_pDevice->GetRecycleBin()->DestroyPipelineLater(pComputePipeline->m_vkPipeline);
	pComputePipeline->m_vkPipeline = nullptr;
}

void GetBlendStage(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo)
{
	blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.logicOp = VK_LOGIC_OP_COPY;

	MEMaterialType eType = pMaterial->GetMaterialType();

	if (MEMaterialType::EDefault == eType || MEMaterialType::EDeferred == eType)
	{
		for (uint32_t i = 0; i < pRenderPass->m_vBackTextures.size(); ++i)
		{
			vBlendAttach.push_back({});
			VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
			attachStage.blendEnable = VK_TRUE;
			attachStage.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachStage.colorBlendOp = VK_BLEND_OP_ADD;
			attachStage.alphaBlendOp = VK_BLEND_OP_ADD;
		}
	}
	else if (MEMaterialType::EDepthPeel == eType)
	{
		if (pRenderPass->m_vBackTextures.size() < 4)
			return;

		vBlendAttach.resize(4);


		{//Front
			vBlendAttach[0].blendEnable = VK_TRUE;
			vBlendAttach[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			vBlendAttach[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			vBlendAttach[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			vBlendAttach[0].colorBlendOp = VK_BLEND_OP_ADD;
			vBlendAttach[0].alphaBlendOp = VK_BLEND_OP_ADD;
		}
		{//Back
			vBlendAttach[1].blendEnable = VK_TRUE;
			vBlendAttach[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			vBlendAttach[1].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			vBlendAttach[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			vBlendAttach[1].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			vBlendAttach[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[1].colorBlendOp = VK_BLEND_OP_ADD;
			vBlendAttach[1].alphaBlendOp = VK_BLEND_OP_ADD;
		}
		{//Front Depth
			vBlendAttach[2].blendEnable = VK_TRUE;
			vBlendAttach[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
			vBlendAttach[2].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[2].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[2].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[2].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[2].colorBlendOp = VK_BLEND_OP_MIN;
			vBlendAttach[2].alphaBlendOp = VK_BLEND_OP_MIN;
		}
		{//Back Depth
			vBlendAttach[3].blendEnable = VK_TRUE;
			vBlendAttach[3].colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
			vBlendAttach[3].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[3].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[3].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[3].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			vBlendAttach[3].colorBlendOp = VK_BLEND_OP_MAX;
			vBlendAttach[3].alphaBlendOp = VK_BLEND_OP_MAX;
		}
	}
	else if (MEMaterialType::ETransparentBlend == eType)
	{
		for (uint32_t i = 0; i < pRenderPass->m_vBackTextures.size(); ++i)
		{
			vBlendAttach.push_back({});
			VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
			attachStage.blendEnable = VK_TRUE;
			attachStage.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachStage.colorBlendOp = VK_BLEND_OP_ADD;
			attachStage.alphaBlendOp = VK_BLEND_OP_MAX;
		}
	}
	else if (MEMaterialType::EImGui == eType)
	{
		for (uint32_t i = 0; i < pRenderPass->m_vBackTextures.size(); ++i)
		{
			vBlendAttach.push_back({});
			VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
			attachStage.blendEnable = VK_TRUE;
			attachStage.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			attachStage.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			attachStage.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachStage.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			attachStage.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			attachStage.colorBlendOp = VK_BLEND_OP_ADD;
			attachStage.alphaBlendOp = VK_BLEND_OP_ADD;
		}

	}

	blendInfo.attachmentCount = vBlendAttach.size();
	blendInfo.pAttachments = vBlendAttach.data();
	blendInfo.blendConstants[0] = 0.0f;
	blendInfo.blendConstants[1] = 0.0f;
	blendInfo.blendConstants[2] = 0.0f;
	blendInfo.blendConstants[3] = 0.0f;

}

void GetDepthStencilStage(std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo)
{
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.pNext = NULL;
	depthStencilInfo.flags = 0;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0;
	depthStencilInfo.maxDepthBounds = 0;
	depthStencilInfo.stencilTestEnable = VK_FALSE;

	MEMaterialType eType = pMaterial->GetMaterialType();

	if (MEMaterialType::EDefault == eType || MEMaterialType::EDeferred == eType)
	{
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
	}
	else if (MEMaterialType::EDepthPeel == eType)
	{
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
	}
	else if (MEMaterialType::ETransparentBlend == eType)
	{
		depthStencilInfo.depthTestEnable = VK_FALSE;
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
	}
	else if (MEMaterialType::EImGui == eType)
	{
		depthStencilInfo = {};
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	}
}

VkPipeline MVulkanPipelineManager::CreateGraphicsPipeline(const std::shared_ptr<MPipeline>& pPipeline, std::shared_ptr<MMaterial> pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx)
{
	if (!pMaterial)
	{
		MORTY_ASSERT(pMaterial);
		return VK_NULL_HANDLE;
	}

	if(pPipeline->m_pipelineLayout.vkPipelineLayout == VK_NULL_HANDLE)
	{
		GeneratePipelineLayout(pPipeline, pMaterial->GetShaderProgram());
	}

	VkPipelineLayout vkPipelineLayout = pPipeline->m_pipelineLayout.vkPipelineLayout;

	if (VK_NULL_HANDLE == vkPipelineLayout)
	{
		MORTY_ASSERT(vkPipelineLayout);
		return VK_NULL_HANDLE;
	}

	MShader* pVertexShader = pMaterial->GetVertexShader();
	MShader* pPixelShader = pMaterial->GetPixelShader();

	if (nullptr == pVertexShader || nullptr == pPixelShader)
		return VK_NULL_HANDLE;

	if (nullptr == pVertexShader->GetBuffer())
		return VK_NULL_HANDLE;
	if (nullptr == pPixelShader->GetBuffer())
		return VK_NULL_HANDLE;


	std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();



	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = UINT_MAX;	//default
	scissor.extent.height = UINT_MAX;

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = 512;
	viewport.height = 512;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineShaderStageCreateInfo vShaderStageInfo[] = {
		pVertexShader->GetBuffer()->m_VkShaderStageInfo,
		pPixelShader->GetBuffer()->m_VkShaderStageInfo,
	};

	MVertexShaderBuffer* pVertexShaderBuffer = static_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer());

	VkPipelineVertexInputStateCreateInfo inputStateInfo = {};
	inputStateInfo = VkPipelineVertexInputStateCreateInfo{};
	inputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputStateInfo.vertexBindingDescriptionCount = pVertexShaderBuffer->m_vBindingDescs.size();
	inputStateInfo.pVertexBindingDescriptions = pVertexShaderBuffer->m_vBindingDescs.data();
	inputStateInfo.vertexAttributeDescriptionCount = pVertexShaderBuffer->m_vAttributeDescs.size();
	inputStateInfo.pVertexAttributeDescriptions = pVertexShaderBuffer->m_vAttributeDescs.data();

	//Rasterization
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0.0f; // Optional
	rasterizationState.depthBiasClamp = 0.0f; // Optional
	rasterizationState.depthBiasSlopeFactor = 0.0f; // Optional

	switch (pMaterial->GetRasterizerType())
	{
	case MERasterizerType::ECullNone:
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		break;

	case MERasterizerType::ECullBack:
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;		//vulkan inverse
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		break;

	case MERasterizerType::ECullFront:
		rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		break;
	case MERasterizerType::EWireframe:
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		break;
	default:
		break;
	}



	VkPipelineColorBlendStateCreateInfo blendInfo = {};
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> vBlendAttach;
	GetBlendStage(pMaterial, pRenderPass, vBlendAttach, blendInfo);

	GetDepthStencilStage(pMaterial, pRenderPass, depthStencilInfo);
	
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f; // Optional
	multisampleState.pSampleMask = nullptr; // Optional
	multisampleState.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampleState.alphaToOneEnable = VK_FALSE; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pStages = vShaderStageInfo;
	pipelineInfo.pVertexInputState = &inputStateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pColorBlendState = &blendInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.layout = vkPipelineLayout;
	pipelineInfo.renderPass = pRenderPass->m_VkRenderPass;
	pipelineInfo.subpass = nSubpassIdx;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	return graphicsPipeline;
}

VkPipeline MVulkanPipelineManager::CreateComputePipeline(const std::shared_ptr<MPipeline>& pPipeline, MComputeDispatcher* pComputeDispatcher)
{
	if (VK_NULL_HANDLE == pPipeline->m_pipelineLayout.vkPipelineLayout)
	{
		GeneratePipelineLayout(pPipeline, pComputeDispatcher->GetShaderProgram());
	}

	VkPipelineLayout vkPipelineLayout = pPipeline->m_pipelineLayout.vkPipelineLayout;
	if (VK_NULL_HANDLE == vkPipelineLayout)
	{
		MORTY_ASSERT(vkPipelineLayout);
		return VK_NULL_HANDLE;
	}


	MShader* pComputeShader = pComputeDispatcher->GetComputeShader();

	if (nullptr == pComputeShader)
		return VK_NULL_HANDLE;

	if (nullptr == pComputeShader->GetBuffer())
		return VK_NULL_HANDLE;

	MShaderBuffer* pComputeShaderBuffer = pComputeShader->GetBuffer();
	
	VkComputePipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = VK_NULL_HANDLE;
	pipelineInfo.flags = 0;
	pipelineInfo.stage = pComputeShaderBuffer->m_VkShaderStageInfo;
	pipelineInfo.layout = vkPipelineLayout;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;

	// TODO 
	// fill pipelineInfo.stage.pSpecializationInfo


	VkPipeline computePipeline;
	if (vkCreateComputePipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	return computePipeline;
}

void MVulkanPipelineManager::GenerateShaderProgram(MShaderProgram* pShaderProgram)
{
	
}

void MVulkanPipelineManager::DestroyShaderProgram(MShaderProgram* pShaderProgram)
{
	
}

void MVulkanPipelineManager::AllocateShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock, const std::shared_ptr<MPipeline>& pPipeline)
{
	std::shared_ptr<MShaderProgram> pShaderProgram = pPropertyBlock->GetShaderProgram();

	if (pPropertyBlock->m_VkDescriptorSet)
	{
		m_pDevice->GetRecycleBin()->DestroyDescriptorSetLater(pPropertyBlock->m_VkDescriptorSet);
		pPropertyBlock->m_VkDescriptorSet = VK_NULL_HANDLE;
	}

	MORTY_ASSERT(VK_NULL_HANDLE == pPropertyBlock->m_VkDescriptorSet);
	MORTY_ASSERT(VK_NULL_HANDLE != pPipeline->m_pipelineLayout.vkPipelineLayout);

	//pPipeline->m_tShaderPropertyBlocks.insert(pPropertyBlock);
	VkDescriptorSetLayout vkDescriptorSetLayout = pPipeline->m_pipelineLayout.vDescriptorSetLayouts[pPropertyBlock->m_unKey];
	MORTY_ASSERT(VK_NULL_HANDLE != vkDescriptorSetLayout);

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

	pPropertyBlock->m_VkDescriptorSet = descriptorSet;
}

void MVulkanPipelineManager::DestroyShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
	std::shared_ptr<MShaderProgram> pShaderProgram = pPropertyBlock->GetShaderProgram();

	DestroyShaderParamSetImpl(pPropertyBlock);
}

void MVulkanPipelineManager::DestroyShaderParamSetImpl(const std::shared_ptr<MShaderPropertyBlock>& pParamSet) const
{
	if (!pParamSet)
		return;

	if (pParamSet->m_VkDescriptorSet)
	{
		m_pDevice->GetRecycleBin()->DestroyDescriptorSetLater(pParamSet->m_VkDescriptorSet);
		pParamSet->m_VkDescriptorSet = VK_NULL_HANDLE;
	}
}

void MVulkanPipelineManager::BindConstantParam(const std::shared_ptr<MShaderConstantParam> pParam, VkWriteDescriptorSet& descriptorWrite)
{
	VkDescriptorBufferInfo& bufferInfo = pParam->m_VkBufferInfo;
	MORTY_ASSERT(VK_NULL_HANDLE != bufferInfo.buffer);

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstBinding = pParam->unBinding;
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional
}

void MVulkanPipelineManager::BindTextureParam(const std::shared_ptr<MShaderTextureParam> pParam, VkWriteDescriptorSet& descriptorWrite)
{
	std::shared_ptr<MTexture> pTexture = pParam->GetTexture();
	if (!pTexture)
	{
		if (pParam->eType == METextureType::ETexture2D)
		{
			pTexture = m_pDevice->m_ShaderDefaultTexture;
		}
		else if (pParam->eType == METextureType::ETextureCube)
		{
			pTexture = m_pDevice->m_ShaderDefaultTextureCube;
		}
		else if (pParam->eType == METextureType::ETexture2DArray)
		{
			pTexture = m_pDevice->m_ShaderDefaultTextureArray;
		}
		else
		{
			MORTY_ASSERT(false);
		}
	}

	if (pTexture)
	{
		VkDescriptorImageInfo& imageInfo = pParam->m_VkImageInfo;
		imageInfo.imageView = pTexture->m_VkImageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = pTexture->m_VkSampler;

		if (VK_NULL_HANDLE == imageInfo.sampler)
		{
			imageInfo.sampler = m_pDevice->m_VkLinearSampler;
		}

		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstBinding = pParam->unBinding;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imageInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		//A VkDescripotrSet can only be updated once on per render. .
	}
}

void MVulkanPipelineManager::BindStorageParam(const std::shared_ptr<MShaderStorageParam> pParam, VkWriteDescriptorSet& descriptorWrite)
{
	const MBuffer* pBuffer = pParam->pBuffer;

	if (!pBuffer)
	{
		MORTY_ASSERT(pBuffer);
		return;
	}

	VkDescriptorBufferInfo& bufferInfo = pParam->m_VkBufferInfo;
	bufferInfo.buffer = pBuffer->m_VkBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = pBuffer->GetSize();

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstBinding = pParam->unBinding;
	descriptorWrite.dstArrayElement = 0;

	descriptorWrite.descriptorType = pParam->m_VkDescriptorType;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

}


#endif
