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
	, m_PipelineManager(pDevice)
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



	InitSemaphores();


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

void MVulkanRenderer::Render(MIRenderTarget* pRenderTarget)
{
	vkWaitForFences(m_pDevice->m_VkDevice, 1, &m_VkInFlightFences, VK_TRUE, UINT64_MAX);
	
	VkFramebuffer frameBuffer = pRenderTarget->GetFrameBuffer(m_unFrameIndex);

	m_VkCommandBuffer = m_pDevice->BeginCommands();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_VkImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;


	submitInfo.commandBufferCount = 1;
	//TODO maybe mutil command buffers for every frame
	submitInfo.pCommandBuffers = &m_VkCommandBuffer;


	VkSemaphore signalSemaphores[] = { m_VkRenderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderTarget->m_VkRenderPass;
	renderPassInfo.framebuffer = frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pRenderTarget->m_VkExtend;

	VkClearValue clearColor = { 0.5f, 0.5f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// Render Datas
	//vkCmdBeginRenderPass(m_VkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	pRenderTarget->OnRender(this);

	//vkCmdEndRenderPass(m_VkCommandBuffer);

	//TODO 不支持多个渲染的嵌套


	m_pDevice->EndCommands(m_VkCommandBuffer);

	vkResetFences(m_pDevice->m_VkDevice, 1, &m_VkInFlightFences);

	if (vkQueueSubmit(m_pDevice->m_VkGraphicsQueue, 1, &submitInfo, m_VkInFlightFences) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void MVulkanRenderer::ClearRenderTargetView(MRenderTextureBuffer* pRenderTextureBuffer, const MColor& color)
{
	VkClearColorValue clearColor = { color.r, color.g, color.b, color.a };
	VkClearValue clearValue = {};
	clearValue.color = clearColor;

	VkImageSubresourceRange imageRange = {};
	imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageRange.levelCount = 1;
	imageRange.layerCount = 1;

	vkCmdClearColorImage(m_VkCommandBuffer, pRenderTextureBuffer->m_VkTextureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);


	// 	VkClearAttachment clearAtts[] = { {VK_IMAGE_ASPECT_COLOR_BIT, 1, {1,0,0,1}} };
	// 	VkClearRect clearRect = { {{0,0}, {1,1}}, 0, 1 };
	// 
	// 	vkCmdClearAttachments(m_VkCommandBuffer, 1, clearAtts, 1, &clearRect);
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

		vkCmdDraw(m_VkCommandBuffer, pMesh->GetVerticesLength(), 1, 0, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}

	
}

bool MVulkanRenderer::SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources /*= false*/)
{
	m_VkUsingPipeline = m_PipelineManager.FindPipeline(pMaterial, m_vRenderTargets.top(), 0);
	m_VkUsingPipelineLayout = m_PipelineManager.FindPipelineLayout(pMaterial);


	vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkUsingPipeline);



	if (bUpdateResources)
	{
		UpdateMaterialResource();
		UpdateMaterialParam();
	}

	return true;
}

void MVulkanRenderer::RegisterMaterial(MMaterial* pMaterial)
{
	m_PipelineManager.RegisterMaterial(pMaterial);
}

void MVulkanRenderer::UnRegisterMaterial(MMaterial* pMaterial)
{
	m_PipelineManager.UnRegisterMaterial(pMaterial);
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



	//Set Use Uniform
	vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkUsingPipelineLayout, 0, 1, &param.m_VkDescriptorSet, 0, nullptr);
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

	VkPipelineLayout pipelineLayout = m_PipelineManager.FindPipelineLayout(pMaterial);

	MIRenderTarget* pCurRenderTarget = m_vRenderTargets.top();

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


#endif