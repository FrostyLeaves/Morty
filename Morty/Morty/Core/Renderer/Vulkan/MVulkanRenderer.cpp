#include "MVulkanRenderer.h"
#include "MMesh.h"
#include "MShader.h"
#include "MMaterial.h"
#include "MVulkanDevice.h"
#include "MIRenderTarget.h"
#include "MRenderStructure.h"
#include "MRenderStatistics.h"


#if RENDER_GRAPHICS == MORTY_VULKAN

MVulkanRenderer::MVulkanRenderer(MVulkanDevice* pDevice)
	: MIRenderer()
	, m_pDevice(pDevice)
	, m_VkUsingPipeline(VK_NULL_HANDLE)
	, m_pUsingMaterial(nullptr)
{

}

MVulkanRenderer::~MVulkanRenderer()
{

}

bool MVulkanRenderer::Initialize()
{


	// Input
	m_InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_InputAssemblyState.primitiveRestartEnable = VK_FALSE;

	//Rasterization
	m_RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_RasterizationState.depthClampEnable = VK_FALSE;
	m_RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	m_RasterizationState.lineWidth = 1.0f;
	m_RasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	m_RasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_RasterizationState.depthBiasEnable = VK_FALSE;
	m_RasterizationState.depthBiasConstantFactor = 0.0f; // Optional
	m_RasterizationState.depthBiasClamp = 0.0f; // Optional
	m_RasterizationState.depthBiasSlopeFactor = 0.0f; // Optional


	//多重采样
	m_MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_MultisampleState.sampleShadingEnable = VK_FALSE;
	m_MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleState.minSampleShading = 1.0f; // Optional
	m_MultisampleState.pSampleMask = nullptr; // Optional
	m_MultisampleState.alphaToCoverageEnable = VK_FALSE; // Optional
	m_MultisampleState.alphaToOneEnable = VK_FALSE; // Optional

	//TODO DepthStencil
//	VkPipelineDepthStencilStateCreateInfo


	//TODO Blend




	InitCommandBuffer();


	return true;
}

void MVulkanRenderer::SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth)
{
	VkViewport viewport = {};
	viewport.x = fX;
	viewport.y = fY;
	viewport.width = fWidth;
	viewport.height = fHeight;
	viewport.minDepth = fMinDepth;
	viewport.maxDepth = fMaxDepth;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = fWidth;
	scissor.extent.height = fHeight;

	m_ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_ViewportState.viewportCount = 1;
	m_ViewportState.pViewports = &viewport;
	m_ViewportState.scissorCount = 1;
	m_ViewportState.pScissors = &scissor;
}


void MVulkanRenderer::DrawMesh(MIMesh* pMesh)
{
	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(m_pDevice);

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(m_pDevice);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkUsingPipeline);

		VkBuffer vertexBuffers[] = { pBuffer->m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_VkCommandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(m_VkCommandBuffer, pMesh->GetVerticesLength(), 1, 0, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}

	
}

bool MVulkanRenderer::SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources /*= false*/)
{
	if (VK_NULL_HANDLE != m_VkUsingPipeline)
	{
		vkDestroyPipeline(m_pDevice->m_VkDevice, m_VkUsingPipeline, nullptr);
	}
	m_VkUsingPipeline = CreateGraphicsPipeline(pMaterial);




	return true;
}

VkPipeline MVulkanRenderer::CreateGraphicsPipeline(MMaterial* pMaterial)
{
	if (!pMaterial)
		return false;

	//Vulkan必须填这个东西，才能在运行时修改viewport大小等设置
	std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	MShader* pVertexShader = pMaterial->GetVertexShader();
	MShader* pPixelShader = pMaterial->GetPixelShader();

	if (nullptr == pVertexShader || nullptr == pPixelShader)
		return false;

	if (nullptr == pVertexShader->GetBuffer())
		return false;
	if (nullptr == pPixelShader->GetBuffer())
		return false;

	VkPipelineShaderStageCreateInfo vShaderStageInfo[] = {
		pVertexShader->GetBuffer()->m_VkShaderStageInfo,
		pPixelShader->GetBuffer()->m_VkShaderStageInfo,
	};

	VkPipelineVertexInputStateCreateInfo inputStateInfo = static_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer())->m_VkVertexInputStateInfo;

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	VkDescriptorSetLayout descriptorSetLayout;

	if (vkCreateDescriptorSetLayout(m_pDevice->m_VkDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}


	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	if (vkCreatePipelineLayout(m_pDevice->m_VkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		return false;

	MIRenderTarget* pCurRenderTarget = m_vRenderTargets.top().pRenderTarget;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = vShaderStageInfo;
	pipelineInfo.pVertexInputState = &inputStateInfo;
	pipelineInfo.pInputAssemblyState = &m_InputAssemblyState;
	pipelineInfo.pViewportState = &m_ViewportState;
	pipelineInfo.pRasterizationState = &m_RasterizationState;
	pipelineInfo.pMultisampleState = &m_MultisampleState;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = nullptr;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pCurRenderTarget->m_VkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return graphicsPipeline;
}

bool MVulkanRenderer::InitCommandBuffer()
{

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_pDevice->m_VkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_pDevice->m_VkDevice, &allocInfo, &m_VkCommandBuffer) != VK_SUCCESS)
		return false;


	return true;
}

#endif