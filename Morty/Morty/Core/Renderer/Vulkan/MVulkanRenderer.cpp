#include "MVulkanRenderer.h"
#include "MMesh.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MVulkanDevice.h"
#include "MMaterialGroup.h"
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
	, m_vRenderStages()

	, m_unFrameIndex(0)
	, m_vRenderPass()
{

}

MVulkanRenderer::~MVulkanRenderer()
{
}

bool MVulkanRenderer::Initialize()
{


	// Input
	m_InputAssemblyState = {};
	m_InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_InputAssemblyState.primitiveRestartEnable = VK_FALSE;



	//���ز���
	m_MultisampleState = {};
	m_MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_MultisampleState.sampleShadingEnable = VK_FALSE;
	m_MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_MultisampleState.minSampleShading = 1.0f; // Optional
	m_MultisampleState.pSampleMask = nullptr; // Optional
	m_MultisampleState.alphaToCoverageEnable = VK_FALSE; // Optional
	m_MultisampleState.alphaToOneEnable = VK_FALSE; // Optional


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
	m_VkViewport.y = fY + fHeight;
	m_VkViewport.width = fWidth;
	m_VkViewport.height = -fHeight;
	m_VkViewport.minDepth = fMinDepth;
	m_VkViewport.maxDepth = fMaxDepth;

	MRenderStage& rs = m_vRenderStages.back();
	vkCmdSetViewport(rs.vkCommandBuffer, 0, 1, &m_VkViewport);
	
	VkRect2D scissorRect = {fX, fY, fWidth, fHeight};
	vkCmdSetScissor(rs.vkCommandBuffer, 0, 1, &scissorRect);
}

void MVulkanRenderer::NewRenderFrame()
{
	m_unFrameIndex = (m_unFrameIndex + 1) % M_BUFFER_NUM;

	VkFence vkInFightFence = m_VkInFlightFences[m_unFrameIndex];

	//if m_VkInFlightFences == signed
	vkWaitForFences(m_pDevice->m_VkDevice, 1, &vkInFightFence, VK_TRUE, UINT64_MAX);

	m_vRenderStages.clear();
}

void MVulkanRenderer::RenderBegin(MIRenderTarget* pRenderTarget)
{
	//�ͷ����õ�Vulkan����
	m_pDevice->m_ObjectDestructor.FrameFinished(m_unFrameIndex);

	m_vRenderStages.push_back(MRenderStage());

//	MLogManager::GetInstance()->Information("begin render frame: %d", m_unFrameIndex);


	//�ͷ���һ��CommandBuffer
	if (pRenderTarget->m_VkCommandBuffers[m_unFrameIndex])
	{
		m_pDevice->m_ObjectDestructor.DestroyCommandBufferLater(pRenderTarget->m_VkCommandBuffers[m_unFrameIndex]);
	}

	//New һ��CommandBuffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_pDevice->m_VkCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(m_pDevice->m_VkDevice, &allocInfo, &pRenderTarget->m_VkCommandBuffers[m_unFrameIndex]);

	//Set Record Using CommandBuffer
	m_vRenderStages.back().vkCommandBuffer = pRenderTarget->m_VkCommandBuffers[m_unFrameIndex];




	//CommandBuffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin Command Buffer
	vkBeginCommandBuffer(m_vRenderStages.back().vkCommandBuffer, &beginInfo);



	//Record Commands
	//pRenderTarget->OnRender(this);

// 	//Process Render Finished Event
// 	if (m_vRenderStages.size() > 1)
// 	{
// 		VkEvent vkEvent = pRenderTarget->m_aVkRenderFinishedEvent[m_unFrameIndex];
// 		
// 		//��һ��RenderTarget�Ľ���������Ⱦ��������Ҫ�ȴ���ǰ����Ⱦ��ɺ��ټ�����
// 		MRenderStage& prs = m_vRenderStages[m_vRenderStages.size() - 2];
// 
// 		std::vector<VkImageMemoryBarrier> vBarriers;
// 		GetRenderTargetBarrier(pRenderTarget, vBarriers);
// 
// 		vkCmdWaitEvents(prs.vkCommandBuffer, 1, &vkEvent
// 			//���Event����֮ǰ�ύ�������������Щ����			//���Event����֮���ύ�������������Щ����
// 			, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
// 			, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE
// 			, vBarriers.size(), vBarriers.data());
// 
// 		//Host Reset and Device Set.
// 		vkResetEvent(m_pDevice->m_VkDevice, vkEvent);
// 		vkCmdSetEvent(m_vRenderStages.back().vkCommandBuffer, vkEvent, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
// 	}

	
}

void MVulkanRenderer::BeginRenderPass(MRenderPass* pRenderPass, MIRenderTarget* pRenderTarget)
{
	if (!pRenderPass || !pRenderTarget)
		return;

	MFrameBuffer* pFrameBuffer = pRenderTarget->GetCurrFrameBuffer(GetFrameIndex());
	if (!pFrameBuffer)
		return;

	if (VK_NULL_HANDLE == pFrameBuffer->vkFrameBuffer)
	{
		if (!m_pDevice->GenerateRenderTarget(pRenderPass, pRenderTarget))
		{
			MLogManager::GetInstance()->Error("MVulkanRenderer::BeginRenderPass error: Generate rt failed.");
			return;
		}
	}

	pFrameBuffer = pRenderTarget->GetCurrFrameBuffer(GetFrameIndex());

	size_t unBackNum = pFrameBuffer->vBackTextures.size();
	if (unBackNum != pRenderPass->m_vBackDesc.size())
		return;


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderPass->m_aVkRenderPass[m_unFrameIndex];
	renderPassInfo.framebuffer = pFrameBuffer->vkFrameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pRenderTarget->m_VkExtend;

	std::vector<VkClearValue> vClearValues(unBackNum);
	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MColor color = pRenderPass->m_vBackDesc[i].cClearColor;
		vClearValues[i].color = { color.r, color.g, color.b, color.a };
	}

	if (pFrameBuffer->pDepthTexture)
	{
		vClearValues.push_back({});
		vClearValues.back().depthStencil = { 1.0f, 0 };
	}

	renderPassInfo.clearValueCount = vClearValues.size();
	renderPassInfo.pClearValues = vClearValues.data();

	//Begin RenderPass
	vkCmdBeginRenderPass(m_vRenderStages.back().vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_vRenderPass.push(pRenderPass);
}

void MVulkanRenderer::EndRenderPass()
{
	m_vRenderPass.pop();

	//End Render Pass
	vkCmdEndRenderPass(m_vRenderStages.back().vkCommandBuffer);




//	vkCmdPipelineBarrier(m_vRenderStages.back().vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,)
}

void MVulkanRenderer::RenderEnd(MIRenderTarget* pRenderTarget)
{
	//End Command Buffer
	vkEndCommandBuffer(m_vRenderStages.back().vkCommandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = pRenderTarget->m_vWaitSemaphoreBeforeSubmit.size();
	submitInfo.pWaitSemaphores = pRenderTarget->m_vWaitSemaphoreBeforeSubmit.data();
	submitInfo.pWaitDstStageMask = waitStages;


	submitInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffers[] = { m_vRenderStages.back().vkCommandBuffer };
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

	m_vRenderStages.pop_back();
}

//����Buffer->��Buffer(����)->��ͼ(����)
void MVulkanRenderer::DrawMesh(MIMesh* pMesh)
{
	MRenderStage& rs = m_vRenderStages.back();

	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(m_pDevice);//�����ʹ����һ��CommandBuffer

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(m_pDevice);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		VkBuffer vertexBuffers[] = { pBuffer->m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(rs.vkCommandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(rs.vkCommandBuffer, pBuffer->m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(rs.vkCommandBuffer, static_cast<uint32_t>(pMesh->GetIndicesLength()), 1, 0, 0, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}
}

//��������->�󶨹���(����)->�󶨲��ʲ���(����)
bool MVulkanRenderer::SetUseMaterial(MMaterial* pMaterial)
{
	if (m_vRenderStages.empty())
		return false;

	if (m_vRenderPass.empty())
		return false;

	MRenderStage& rs = m_vRenderStages.back();

	if (nullptr == pMaterial)
	{
		rs.pUsingMaterial = nullptr;
		rs.pUsingPipelineLayoutData = nullptr;
		return true;
	}

	MRenderPass* pRenderPass = m_vRenderPass.top();
	MMaterialPipelineLayoutData* pPipelineLayoutData = m_pDevice->m_PipelineManager.FindOrCreatePipelineLayout(pMaterial);

	if (rs.pUsingPipelineLayoutData == pPipelineLayoutData)
		return true;

	rs.pUsingMaterial = pMaterial;
	rs.pUsingPipelineLayoutData = pPipelineLayoutData;

	VkPipeline vkPipeline = m_pDevice->m_PipelineManager.FindPipeline(pMaterial, pRenderPass);
	if (!vkPipeline)
	{
		vkPipeline = CreateGraphicsPipeline(pMaterial, pRenderPass);
		m_pDevice->m_PipelineManager.SetPipeline(pMaterial, pRenderPass, vkPipeline);
	}
	
	if (vkPipeline)
	{
		vkCmdBindPipeline(rs.vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		MShaderParamSet* pParamSet = pMaterial->GetMaterialParamSet();
		SetShaderParamSet(pParamSet);

		return true;
	}

	return false;
}

void MVulkanRenderer::GetBlendStage(MMaterial* pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo)
{
	

	blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.logicOp = VK_LOGIC_OP_COPY;

	MEMaterialType eType = pMaterial->GetMaterialType();

	if (MEMaterialType::EDefault == eType)
	{
		//m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EDefault], 0);

		for (uint32_t i = 0; i < pRenderPass->m_vBackDesc.size(); ++i)
		{
			vBlendAttach.push_back({});
			VkPipelineColorBlendAttachmentState& attachStage = vBlendAttach.back();
			attachStage.blendEnable = VK_FALSE;
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
	else if (MEMaterialType::ETransparent == eType)
	{
	//	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EReadNotWrite], 0);
		
		if (pRenderPass->m_vBackDesc.size() != 4)
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
	else if (MEMaterialType::EBlendTransparent == eType)
	{
		//m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::ENotReadNotWrite], 0);
		for (uint32_t i = 0; i < pRenderPass->m_vBackDesc.size(); ++i)
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

	blendInfo.attachmentCount = vBlendAttach.size();
	blendInfo.pAttachments = vBlendAttach.data();
	blendInfo.blendConstants[0] = 0.0f;
	blendInfo.blendConstants[1] = 0.0f;
	blendInfo.blendConstants[2] = 0.0f;
	blendInfo.blendConstants[3] = 0.0f;

}

void MVulkanRenderer::GetDepthStencilStage(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo)
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

	if (MEMaterialType::EDefault == eType)
	{
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
	}
	else if (MEMaterialType::ETransparent == eType)
	{
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_FALSE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
	}
	else if (MEMaterialType::EBlendTransparent == eType)
	{
		//m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::ENotReadNotWrite], 0);
		for (uint32_t i = 0; i < pRenderPass->m_vBackDesc.size(); ++i)
		{
			depthStencilInfo.depthTestEnable = VK_FALSE;
			depthStencilInfo.depthWriteEnable = VK_FALSE;
			depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilInfo.stencilTestEnable = VK_FALSE;
		}
	}
}

void MVulkanRenderer::UpdateShaderParam(MShaderConstantParam& param, const uint32_t& unFrameIdx)
{
	if (VK_NULL_HANDLE == param.m_VkBuffer)
		return;

	if (param.bDirty && param.m_pMemoryMapping[unFrameIdx])
	{
		memcpy(param.m_pMemoryMapping[unFrameIdx] + param.m_unMemoryOffset[unFrameIdx], param.var.GetData(), param.var.GetSize());

		//TODO android 或其它平台可能需要手动刷新一下缓存
// 		VkMappedMemoryRange memoryRange = {};
// 		memoryRange.sType = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
// 		memoryRange.memory = param.m_VkBufferMemory[m_unFrameIndex];
// 		memoryRange.offset = param.m_unMemoryOffset[m_unFrameIndex];
// 		memoryRange.size = param.m_unVkMemorySize;
// 		vkFlushMappedMemoryRanges(m_pDevice->m_VkDevice, 1, &memoryRange);

		param.bDirty[unFrameIdx] = false;
	}

}

void MVulkanRenderer::SetShaderParamSet(MShaderParamSet* pParamSet)
{
	MRenderStage& rs = m_vRenderStages.back();

	MMaterialPipelineLayoutData* pLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
	{
		if (!rs.pUsingMaterial)
			return;
		if (!rs.pUsingPipelineLayoutData)
			return;

		pParamSet->m_nDescriptorSetInitMaterialIdx = rs.pUsingMaterial->GetMaterialID();

		pParamSet->GenerateBuffer(m_pDevice);
	}

	std::vector<uint32_t> vDynamicOffsets;

	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		UpdateShaderParam(*pParam, m_unFrameIndex);

		if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			vDynamicOffsets.push_back(pParam->m_unMemoryOffset[m_unFrameIndex]);
		}
	}

	for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
	{
		if (pParam->bDirty[m_unFrameIndex])
		{
			if (pParam->pTexture && !pParam->pTexture->GetBuffer())
			{
				continue;
			}

			m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, pParam, m_unFrameIndex);
			pParam->bDirty[m_unFrameIndex] = false;
		}
	}


	vkCmdBindDescriptorSets(rs.vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rs.pUsingPipelineLayoutData->pipelineLayout, pParamSet->m_unKey, 1, &pParamSet->m_VkDescriptorSet[m_unFrameIndex], vDynamicOffsets.size(), vDynamicOffsets.data());
}

VkPipeline MVulkanRenderer::CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass)
{
	if (!pMaterial)
		return VK_NULL_HANDLE;

	MShader* pVertexShader = pMaterial->GetVertexShader();
	MShader* pPixelShader = pMaterial->GetPixelShader();

	if (nullptr == pVertexShader || nullptr == pPixelShader)
		return VK_NULL_HANDLE;

	if (nullptr == pVertexShader->GetBuffer())
		return VK_NULL_HANDLE;
	if (nullptr == pPixelShader->GetBuffer())
		return VK_NULL_HANDLE;


	//Vulkan�������������������������ʱ�޸�viewport��С������
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
	scissor.extent.width = m_VkViewport.width;	//TODO
	scissor.extent.height = m_VkViewport.height;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &m_VkViewport;
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

	//��vkCreateGraphicsPipelines֮ǰ��vBlendAttach���ܱ��ͷ�
	std::vector<VkPipelineColorBlendAttachmentState> vBlendAttach;
	GetBlendStage(pMaterial, pRenderPass, vBlendAttach, blendInfo);

	GetDepthStencilStage(pMaterial, pRenderPass, depthStencilInfo);

	VkPipelineLayout pipelineLayout = m_pDevice->m_PipelineManager.FindOrCreatePipelineLayout(pMaterial)->pipelineLayout;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.pStages = vShaderStageInfo;
	pipelineInfo.pVertexInputState = &inputStateInfo;
	pipelineInfo.pInputAssemblyState = &m_InputAssemblyState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizationState;
	pipelineInfo.pMultisampleState = &m_MultisampleState;
	pipelineInfo.pColorBlendState = &blendInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pRenderPass->m_aVkRenderPass[m_unFrameIndex];
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return graphicsPipeline;
}

void MVulkanRenderer::GetRenderTargetBarrier(MIRenderTarget* pRenderTarget, std::vector<VkImageMemoryBarrier>& vResult)
{
	MFrameBuffer* pFrameBuffer = pRenderTarget->GetCurrFrameBuffer(m_unFrameIndex);
	if (!pFrameBuffer)
		return;

	int nBackNum = pFrameBuffer->vBackTextures.size();
	for (int i = 0; i < nBackNum; ++i)
	{
		if (MIRenderBackTexture* pBackTexture = pFrameBuffer->vBackTextures[i])
		{
			MRenderTextureBuffer* pBuffer =  pBackTexture->GetRenderBuffer();

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = pBuffer->m_VkImageLayout;
			barrier.newLayout = pBuffer->m_VkImageLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pBuffer->m_VkTextureImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vResult.push_back(barrier);
		}

	}
	if (MRenderDepthTexture* pDepthTexture = pFrameBuffer->pDepthTexture)
	{
		if (MDepthTextureBuffer* pBuffer = pDepthTexture->GetDepthBuffer())
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = pBuffer->m_VkImageLayout;
			barrier.newLayout = pBuffer->m_VkImageLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = pBuffer->m_VkTextureImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			vResult.push_back(barrier);
		}
	}

}

bool MVulkanRenderer::InitSemaphores()
{
	


	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < m_VkInFlightFences.size(); ++i)
		vkCreateFence(m_pDevice->m_VkDevice, &fenceInfo, nullptr, &m_VkInFlightFences[i]);

	return true;
}


void MVulkanRenderer::ReleaseSemaphores()
{
	for (uint32_t i = 0; i < m_VkInFlightFences.size(); ++i)
	{
		vkDestroyFence(m_pDevice->m_VkDevice, m_VkInFlightFences[i], nullptr);
		m_VkInFlightFences[i] = VK_NULL_HANDLE;
	}
}


VkCommandBuffer MVulkanRenderer::GetCommandBuffer()
{
	if (m_vRenderStages.empty())
		return VK_NULL_HANDLE;

	return m_vRenderStages.back().vkCommandBuffer;
}

#endif