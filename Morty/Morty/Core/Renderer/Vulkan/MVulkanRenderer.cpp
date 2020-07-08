#include "MVulkanRenderer.h"
#include "MMesh.h"
#include "MShader.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MVulkanDevice.h"
#include "MIRenderTarget.h"
#include "MRenderStructure.h"
#include "MRenderStatistics.h"

#include "MVulkanRenderTarget.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

MVulkanRenderer::MVulkanRenderer(MVulkanDevice* pDevice)
	: MIRenderer()
	, m_pDevice(pDevice)
	, m_VkUsingPipeline(VK_NULL_HANDLE)
	, m_pUsingMaterial(nullptr)

	, m_VkCommandBuffer(VK_NULL_HANDLE)

	, m_VkImageAvailableSemaphore(VK_NULL_HANDLE)
	, m_VkRenderFinishedSemaphore(VK_NULL_HANDLE)

	, m_unFrameIndex(0)
{

}

MVulkanRenderer::~MVulkanRenderer()
{

}

void MVulkanRenderer::AddOutputView(MIRenderView* pView)
{
	if (MWindowsRenderView* pWindowView = dynamic_cast<MWindowsRenderView*>(pView))
	{
		MVulkanRenderTarget::CreateForWindowsView(m_pDevice, pWindowView);
	}
}

bool MVulkanRenderer::Initialize()
{


	// Input
	m_InputAssemblyState = {};
	m_InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_InputAssemblyState.primitiveRestartEnable = VK_FALSE;

	//Rasterization
	m_RasterizationState = {};
	m_RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_RasterizationState.depthClampEnable = VK_FALSE;
	m_RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	m_RasterizationState.lineWidth = 1.0f;
	m_RasterizationState.cullMode = VK_CULL_MODE_NONE;
	m_RasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_RasterizationState.depthBiasEnable = VK_FALSE;
	m_RasterizationState.depthBiasConstantFactor = 0.0f; // Optional
	m_RasterizationState.depthBiasClamp = 0.0f; // Optional
	m_RasterizationState.depthBiasSlopeFactor = 0.0f; // Optional


	//多重采样
	m_MultisampleState = {};
	m_MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_MultisampleState.sampleShadingEnable = VK_FALSE;
	m_MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleState.minSampleShading = 1.0f; // Optional
	m_MultisampleState.pSampleMask = nullptr; // Optional
	m_MultisampleState.alphaToCoverageEnable = VK_FALSE; // Optional
	m_MultisampleState.alphaToOneEnable = VK_FALSE; // Optional

	//TODO DepthStencil
//	VkPipelineDepthStencilStateCreateInfo


	m_ColorBlendAttachment = {};
	m_ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_ColorBlendAttachment.blendEnable = VK_FALSE;

	m_ColorBlending = {};
	m_ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_ColorBlending.logicOpEnable = VK_FALSE;
	m_ColorBlending.logicOp = VK_LOGIC_OP_COPY;
	m_ColorBlending.attachmentCount = 1;
	m_ColorBlending.pAttachments = &m_ColorBlendAttachment;
	m_ColorBlending.blendConstants[0] = 0.0f;
	m_ColorBlending.blendConstants[1] = 0.0f;
	m_ColorBlending.blendConstants[2] = 0.0f;
	m_ColorBlending.blendConstants[3] = 0.0f;


	InitSemaphores();


	return true;
}

void MVulkanRenderer::Release()
{
	m_pDevice->m_PipelineManager.Release();

	while (vkGetFenceStatus(m_pDevice->m_VkDevice, m_VkInFlightFences) != VK_SUCCESS);

	if (m_VkCommandBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_pDevice->m_VkDevice, m_pDevice->m_VkCommandPool, 1, &m_VkCommandBuffer);
		m_VkCommandBuffer = VK_NULL_HANDLE;
	}

	m_pDevice->m_BufferManager.FrameFinished(0);

	ReleaseSemaphores();

}

void MVulkanRenderer::SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth)
{
	m_VkViewport = {};
	m_VkViewport.x = fX;
	m_VkViewport.y = fY;
	m_VkViewport.width = fWidth;
	m_VkViewport.height = fHeight;
	m_VkViewport.minDepth = fMinDepth;
	m_VkViewport.maxDepth = fMaxDepth;

	vkCmdSetViewport(m_VkCommandBuffer, 0, 1, &m_VkViewport);
}

void MVulkanRenderer::Render(MIRenderTarget* pRenderTarget)
{
	//if m_VkInFlightFences == signed
	vkWaitForFences(m_pDevice->m_VkDevice, 1, &m_VkInFlightFences, VK_TRUE, UINT64_MAX);

	//TODO check render end.
	m_pDevice->m_BufferManager.FrameFinished(0);

	if (m_VkCommandBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_pDevice->m_VkDevice, m_pDevice->m_VkCommandPool, 1, &m_VkCommandBuffer);
		m_VkCommandBuffer = VK_NULL_HANDLE;
	}

	pRenderTarget->OnRenderBefore(this);

	VkFramebuffer frameBuffer = pRenderTarget->GetFrameBuffer(m_unFrameIndex);



	//m_VkCommandBuffer = m_pDevice->BeginCommands();
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_pDevice->m_VkCommandPool;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(m_pDevice->m_VkDevice, &allocInfo, &m_VkCommandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderTarget->m_RenderPass.m_VkRenderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pRenderTarget->m_VkExtend;

	uint32_t unBackNum = pRenderTarget->GetBackNum();
	std::vector<VkClearValue> vClearValues(unBackNum);
	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MColor color = pRenderTarget->GetBackClearColor(i);
		vClearValues[i] = {color.r, color.g, color.b, color.a};
	}

	renderPassInfo.clearValueCount = vClearValues.size();
	renderPassInfo.pClearValues = vClearValues.data();

	// Render Datas
	vkCmdBeginRenderPass(m_VkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_vRenderTargets.push(pRenderTarget);
	pRenderTarget->OnRender(this);
	m_vRenderTargets.pop();

	vkCmdEndRenderPass(m_VkCommandBuffer);

	//TODO 不支持多个渲染的嵌套


	//m_pDevice->EndCommands(m_VkCommandBuffer);
	vkEndCommandBuffer(m_VkCommandBuffer);




	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_VkImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;


	submitInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffers[] = { m_VkCommandBuffer };
	//TODO maybe mutil command buffers for every frame
	submitInfo.pCommandBuffers = commandBuffers;


	VkSemaphore signalSemaphores[] = { m_VkRenderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	//m_VkInFlightFences = unsigned
	vkResetFences(m_pDevice->m_VkDevice, 1, &m_VkInFlightFences);
	
	if (vkQueueSubmit(m_pDevice->m_VkGraphicsQueue, 1, &submitInfo, m_VkInFlightFences) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	pRenderTarget->OnRenderAfter(this);


}

void MVulkanRenderer::DrawMesh(MIMesh* pMesh)
{
	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(m_pDevice);

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(m_pDevice);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		VkBuffer vertexBuffers[] = { pBuffer->m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_VkCommandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(m_VkCommandBuffer, pBuffer->m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_VkCommandBuffer, static_cast<uint32_t>(pMesh->GetIndicesLength()), 1, 0, 0, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}

	
}

bool MVulkanRenderer::SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources /*= false*/)
{
	MRenderPass* pRenderPass = &(m_vRenderTargets.top()->m_RenderPass);
	m_VkUsingPipelineLayout = m_pDevice->m_PipelineManager.FindPipelineLayout(pMaterial);

	m_VkUsingPipeline = m_pDevice->m_PipelineManager.FindPipeline(pMaterial, pRenderPass);
	if (!m_VkUsingPipeline)
	{
		m_VkUsingPipeline = CreateGraphicsPipeline(pMaterial, pRenderPass);
		m_pDevice->m_PipelineManager.SetPipeline(pMaterial, pRenderPass, m_VkUsingPipeline);
	}
	
	if (m_VkUsingPipeline)
	{
		m_pUsingMaterial = pMaterial;
		vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkUsingPipeline);

		if (bUpdateResources)
		{
			UpdateMaterialResource();
			UpdateMaterialParam();
		}

		return true;
	}

	return false;
}

void MVulkanRenderer::UpdateMaterialParam()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderParam& param : *m_pUsingMaterial->GetShaderParams())
	{
		SetShaderParam(param);
	}
}

void MVulkanRenderer::UpdateMaterialResource()
{
	if (!m_pUsingMaterial)
		return;

	for (MShaderTextureParam& param : *m_pUsingMaterial->GetTextureParams())
	{
		SetShaderTexture(param);
	}
}

void MVulkanRenderer::UpdateShaderParam(MShaderParam& param)
{
	if (VK_NULL_HANDLE == param.m_VkBuffer)
		return;

	uint32_t unSize = param.var.GetSize();

	// Update Memory
	void* data = nullptr;
	vkMapMemory(m_pDevice->m_VkDevice, param.m_VkBufferMemory, 0, unSize, 0, &data);
	memcpy(data, param.var.GetData(), unSize);
	vkUnmapMemory(m_pDevice->m_VkDevice, param.m_VkBufferMemory);


	// Update Uniform
	//MShaderParam param;
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




	param.bDirty = false;
}

void MVulkanRenderer::SetShaderParam(MShaderParam& param)
{
	if (param.bDirty)
		UpdateShaderParam(param);

	if (m_VkUsingPipelineLayout != VK_NULL_HANDLE)
	{
		//Set Use Uniform
		vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkUsingPipelineLayout, 0, 1, &param.m_VkDescriptorSet, 0, nullptr);
	}
}

void MVulkanRenderer::SetShaderTexture(MShaderTextureParam& param)
{
}

VkPipeline MVulkanRenderer::CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass)
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



	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = m_VkViewport.width;	//TODO
	scissor.extent.height = m_VkViewport.height;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &m_VkViewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;





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

	MVertexShaderBuffer* pVertexShaderBuffer = static_cast<MVertexShaderBuffer*>(pVertexShader->GetBuffer());

	VkPipelineVertexInputStateCreateInfo inputStateInfo = {};
	inputStateInfo = VkPipelineVertexInputStateCreateInfo{};
	inputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputStateInfo.vertexBindingDescriptionCount = pVertexShaderBuffer->m_vBindingDescs.size();
	inputStateInfo.pVertexBindingDescriptions = pVertexShaderBuffer->m_vBindingDescs.data();
	inputStateInfo.vertexAttributeDescriptionCount = pVertexShaderBuffer->m_vAttributeDescs.size();
	inputStateInfo.pVertexAttributeDescriptions = pVertexShaderBuffer->m_vAttributeDescs.data();
	
	VkPipelineLayout pipelineLayout = m_pDevice->m_PipelineManager.FindPipelineLayout(pMaterial);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pStages = vShaderStageInfo;
	pipelineInfo.pVertexInputState = &inputStateInfo;
	pipelineInfo.pInputAssemblyState = &m_InputAssemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &m_RasterizationState;
	pipelineInfo.pMultisampleState = &m_MultisampleState;
	pipelineInfo.pColorBlendState = &m_ColorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pRenderPass->m_VkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return graphicsPipeline;
}

bool MVulkanRenderer::InitSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(m_pDevice->m_VkDevice, &semaphoreInfo, nullptr, &m_VkImageAvailableSemaphore) != VK_SUCCESS)
		return false;

	if (vkCreateSemaphore(m_pDevice->m_VkDevice, &semaphoreInfo, nullptr, &m_VkRenderFinishedSemaphore) != VK_SUCCESS)
		return false;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	if (vkCreateFence(m_pDevice->m_VkDevice, &fenceInfo, nullptr, &m_VkInFlightFences) != VK_SUCCESS)
		return false;

	return true;
}


void MVulkanRenderer::ReleaseSemaphores()
{
	vkDestroySemaphore(m_pDevice->m_VkDevice, m_VkImageAvailableSemaphore, nullptr);
	vkDestroySemaphore(m_pDevice->m_VkDevice, m_VkRenderFinishedSemaphore, nullptr);

	vkDestroyFence(m_pDevice->m_VkDevice, m_VkInFlightFences, nullptr);
}

#endif