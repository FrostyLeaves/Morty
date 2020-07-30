#include "MVulkanRenderer.h"
#include "MMesh.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MVulkanDevice.h"
#include "MIRenderTarget.h"
#include "Shader/MShader.h"
#include "MRenderStructure.h"
#include "MRenderStatistics.h"
#include "Shader/MShaderBuffer.h"

#include "MVulkanRenderTarget.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

MVulkanRenderer::MVulkanRenderer(MVulkanDevice* pDevice)
	: MIRenderer()
	, m_pDevice(pDevice)
	, m_pUsingPipelineLayoutData(nullptr)

	, m_VkCommandBuffer(VK_NULL_HANDLE)

	, m_VkImageAvailableSemaphore(VK_NULL_HANDLE)

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
	m_DepthStencilState = {};
	m_DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_DepthStencilState.pNext = NULL;
	m_DepthStencilState.flags = 0;
	m_DepthStencilState.depthTestEnable = VK_TRUE;
	m_DepthStencilState.depthWriteEnable = VK_TRUE;
	m_DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_DepthStencilState.depthBoundsTestEnable = VK_FALSE;
	m_DepthStencilState.minDepthBounds = 0;
	m_DepthStencilState.maxDepthBounds = 0;
	m_DepthStencilState.stencilTestEnable = VK_FALSE;
	m_DepthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	m_DepthStencilState.back.compareMask = 0;
	m_DepthStencilState.back.reference = 0;
	m_DepthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
	m_DepthStencilState.back.writeMask = 0;
	m_DepthStencilState.front = m_DepthStencilState.back;

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
	//All RenderTarget finished all renderpass.
	for (VkFence vkFence : m_VkInFlightFences)
	{
		while (vkGetFenceStatus(m_pDevice->m_VkDevice, vkFence) != VK_SUCCESS);
	}

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

void MVulkanRenderer::NewRenderFrame()
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if(vkGetFenceStatus(m_pDevice->m_VkDevice, m_VkInFlightFences[m_unFrameIndex]) == VK_SUCCESS)
			MLogManager::GetInstance()->Information("finish render frame: %d", i);
	}

	m_unFrameIndex = (m_unFrameIndex + 1) % M_BUFFER_NUM;

	//渲染栅栏，防止上一次渲染还没渲染完，就执行了下一次的渲染
	VkFence vkInFightFence = m_VkInFlightFences[m_unFrameIndex];

	//if m_VkInFlightFences == signed
	vkWaitForFences(m_pDevice->m_VkDevice, 1, &vkInFightFence, VK_TRUE, UINT64_MAX);
}

void MVulkanRenderer::Render(MIRenderTarget* pRenderTarget)
{
	MLogManager::GetInstance()->Information("begin render frame: %d", m_unFrameIndex);

	//Unused CommandBuffer
	if (pRenderTarget->m_VkCommandBuffers[m_unFrameIndex])
	{
		m_pDevice->m_ObjectDestructor.DestroyCommandBufferLater(m_unFrameIndex, pRenderTarget->m_VkCommandBuffers[m_unFrameIndex]);
	}

	//Destroy All VulkanObject
	m_pDevice->m_ObjectDestructor.FrameFinished(m_unFrameIndex);

	//Use new CommandBuffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_pDevice->m_VkCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(m_pDevice->m_VkDevice, &allocInfo, &pRenderTarget->m_VkCommandBuffers[m_unFrameIndex]);


	//Set Record Using CommandBuffer
	m_VkCommandBuffer = pRenderTarget->m_VkCommandBuffers[m_unFrameIndex];


	//渲染用的Frame Buffer
	VkFramebuffer vkFrameBuffer = pRenderTarget->GetFrameBuffer(m_unFrameIndex);


	//CommandBuffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderTarget->m_RenderPass.m_aVkRenderPass[m_unFrameIndex];
	renderPassInfo.framebuffer = vkFrameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pRenderTarget->m_VkExtend;

	//清除上一帧的渲染数据
	uint32_t unBackNum = pRenderTarget->GetBackNum();
	std::vector<VkClearValue> vClearValues(unBackNum);
	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MColor color = pRenderTarget->GetBackClearColor(i);
		vClearValues[i].color = {color.r, color.g, color.b, color.a};
	}

	if (MRenderDepthTexture* pDepthTexture = pRenderTarget->GetDepthTexture())
	{
		if (MDepthTextureBuffer* pBuffer = pDepthTexture->GetDepthBuffer())
		{
			vClearValues.push_back({});
			vClearValues.back().depthStencil = { 1.0f, 0 };
		}
	}

	renderPassInfo.clearValueCount = vClearValues.size();
	renderPassInfo.pClearValues = vClearValues.data();


	//Begin Command Buffer
	vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo);

	pRenderTarget->OnRenderBefore(this);

	//Begin RenderPass
	vkCmdBeginRenderPass(m_VkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	m_pUsingPipelineLayoutData = nullptr;

	//Record Commands
	m_vRenderTargets.push(pRenderTarget);
	pRenderTarget->OnRender(this);
	m_vRenderTargets.pop();

	//End Render Pass
	vkCmdEndRenderPass(m_VkCommandBuffer);


	//End Command Buffer
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

	VkSemaphore signalSemaphores[] = { pRenderTarget->m_aVkRenderFinishedSemaphore[m_unFrameIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkFence vkInFightFence = m_VkInFlightFences[m_unFrameIndex];
	//m_VkInFlightFences = unsigned
	vkResetFences(m_pDevice->m_VkDevice, 1, &vkInFightFence);
	if (vkQueueSubmit(m_pDevice->m_VkGraphicsQueue, 1, &submitInfo, vkInFightFence) != VK_SUCCESS) {
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

bool MVulkanRenderer::SetUseMaterial(MMaterial* pMaterial)
{
	if (nullptr == pMaterial)
	{
		m_pUsingPipelineLayoutData = nullptr;
		return true;
	}

	MRenderPass* pRenderPass = &(m_vRenderTargets.top()->m_RenderPass);
	MMaterialPipelineLayoutData* pPipelineLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pMaterial);

	if (m_pUsingPipelineLayoutData == pPipelineLayoutData)
		return true;

	m_pUsingPipelineLayoutData = pPipelineLayoutData;

	VkPipeline vkPipeline = m_pDevice->m_PipelineManager.FindPipeline(pMaterial, pRenderPass);
	if (!vkPipeline)
	{
		vkPipeline = CreateGraphicsPipeline(pMaterial, pRenderPass);
		m_pDevice->m_PipelineManager.SetPipeline(pMaterial, pRenderPass, vkPipeline);
	}
	
	if (vkPipeline)
	{
		vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		MShaderParamSet* pParamSet = pMaterial->GetMaterialParamSet();
		SetShaderParamSet(pParamSet);

		return true;
	}

	return false;
}

void MVulkanRenderer::UpdateShaderParam(MShaderConstantParam& param)
{
	if (VK_NULL_HANDLE == param.m_VkBuffer)
		return;

	if (param.m_pMemoryMapping[m_unFrameIndex])
	{
		memcpy(param.m_pMemoryMapping[m_unFrameIndex] + param.m_unMemoryOffset[m_unFrameIndex], param.var.GetData(), param.var.GetSize());

		//TODO 安卓可能有问题 
// 		VkMappedMemoryRange memoryRange = {};
// 		memoryRange.sType = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
// 		memoryRange.memory = param.m_VkBufferMemory[m_unFrameIndex];
// 		memoryRange.offset = param.m_unMemoryOffset[m_unFrameIndex];
// 		memoryRange.size = param.m_unVkMemorySize;
// 		vkFlushMappedMemoryRanges(m_pDevice->m_VkDevice, 1, &memoryRange);
	}

	param.bDirty[m_unFrameIndex] = false;
}

void MVulkanRenderer::SetShaderParamSet(MShaderParamSet* pParamSet)
{
	if (pParamSet->m_VkDescriptorSet[m_unFrameIndex] == VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
			pParamSet->m_VkDescriptorSet[i] = m_pDevice->m_PipelineManager.CreateMaterialDescriptorSet(*m_pUsingPipelineLayoutData, pParamSet->m_unKey);
	}

	std::vector<uint32_t> vDynamicOffsets;

	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		//TODO 不优雅
		if (pParam->m_VkBuffer[m_unFrameIndex] == VK_NULL_HANDLE)
		{
			m_pDevice->GenerateShaderParamBuffer(pParam);
			
			for(uint32_t i = 0; i < M_BUFFER_NUM; ++i)
				m_pDevice->m_PipelineManager.BindConstantParam(pParamSet, pParam, i);
		}

		UpdateShaderParam(*pParam);

		if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			vDynamicOffsets.push_back(pParam->m_unMemoryOffset[m_unFrameIndex]);
		}
	}

	for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
	{
		if (pParam->bDirty[m_unFrameIndex])
		{
			if(pParam->pTexture && !pParam->pTexture->GetBuffer())
				pParam->pTexture->GenerateBuffer(m_pDevice, false);


			m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, pParam, m_unFrameIndex);
			pParam->bDirty[m_unFrameIndex] = false;
		}
	}


	vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pUsingPipelineLayoutData->pipelineLayout, pParamSet->m_unKey, 1, &pParamSet->m_VkDescriptorSet[m_unFrameIndex], vDynamicOffsets.size(), vDynamicOffsets.data());
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
	
	VkPipelineLayout pipelineLayout = m_pDevice->m_PipelineManager.FindPipelineLayout(pMaterial)->pipelineLayout;

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
	pipelineInfo.pDepthStencilState = &m_DepthStencilState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pRenderPass->m_aVkRenderPass[m_unFrameIndex];
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



	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < m_VkInFlightFences.size(); ++i)
		vkCreateFence(m_pDevice->m_VkDevice, &fenceInfo, nullptr, &m_VkInFlightFences[i]);

	return true;
}


void MVulkanRenderer::ReleaseSemaphores()
{
	vkDestroySemaphore(m_pDevice->m_VkDevice, m_VkImageAvailableSemaphore, nullptr);

	for (uint32_t i = 0; i < m_VkInFlightFences.size(); ++i)
	{
		vkDestroyFence(m_pDevice->m_VkDevice, m_VkInFlightFences[i], nullptr);
		m_VkInFlightFences[i] = VK_NULL_HANDLE;
	}
}


#endif