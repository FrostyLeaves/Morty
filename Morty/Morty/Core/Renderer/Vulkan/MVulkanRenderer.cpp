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
	, m_InputAssemblyState({})
	, m_MultisampleState({})
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

	return true;
}

void MVulkanRenderer::Release()
{

}

void MVulkanRenderer::SetViewport(MRenderCommand* pCommand, const MViewportInfo& viewport)
{
	if (!pCommand)
		return;

	VkViewport vkViewport = {};
	vkViewport.x = viewport.x;
	vkViewport.y = viewport.y + viewport.height;
	vkViewport.width = viewport.width;
	vkViewport.height = -viewport.height;
	vkViewport.minDepth = viewport.minz;
	vkViewport.maxDepth = viewport.maxz;

	vkCmdSetViewport(pCommand->m_VkCommandBuffer, 0, 1, &vkViewport);

}

void MVulkanRenderer::SetScissor(MRenderCommand* pCommand, const MScissorInfo& scissor)
{
	if (!pCommand)
		return;

	VkRect2D scissorRect = { int32_t(scissor.x), int32_t(scissor.y), uint32_t(scissor.width), uint32_t(scissor.height) };
	vkCmdSetScissor(pCommand->m_VkCommandBuffer, 0, 1, &scissorRect);
}

void MVulkanRenderer::RenderCommandBegin(MRenderCommand* pCommand)
{
	{//TODO more elegant.
		m_pDevice->m_ObjectDestructor.FrameFinished(pCommand->m_unFrameIdx);

		//if m_VkInFlightFences == signed
		vkWaitForFences(m_pDevice->m_VkDevice, 1, &pCommand->m_VkRenderFinishedFence, VK_TRUE, UINT64_MAX);

		for (auto callback : pCommand->m_aRenderFinishedCallback)
		{
			if (callback)
			{
				callback();
			}
		}

		pCommand->m_aRenderFinishedCallback.clear();
	}

	vkResetCommandBuffer(pCommand->m_VkCommandBuffer, 0);


	//CommandBuffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin Command Buffer
	vkBeginCommandBuffer(pCommand->m_VkCommandBuffer, &beginInfo);
}

void MVulkanRenderer::RenderCommandEnd(MRenderCommand* pCommand)
{
	if (!pCommand || !pCommand->m_VkCommandBuffer)
		return;

	//End Command Buffer
	vkEndCommandBuffer(pCommand->m_VkCommandBuffer);
}

void MVulkanRenderer::SubmitRenderCommand(MRenderCommand* pCommand, MIRenderTarget* pRenderTarget)
{
	if (!pCommand || !pCommand->m_VkCommandBuffer)
		return;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


	if (pRenderTarget)
	{
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = pRenderTarget->m_vWaitSemaphoreBeforeSubmit.size();
		submitInfo.pWaitSemaphores = pRenderTarget->m_vWaitSemaphoreBeforeSubmit.data();
		submitInfo.pWaitDstStageMask = waitStages;
	}


	submitInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffers[] = { pCommand->m_VkCommandBuffer };
	//TODO maybe mutil command buffers for every frame
	submitInfo.pCommandBuffers = commandBuffers;


	VkSemaphore signalSemaphores[] = { pCommand->m_VkRenderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	

	VkFence vkInFightFence = pCommand->m_VkRenderFinishedFence;
	//m_VkInFlightFences = unsigned
	vkResetFences(m_pDevice->m_VkDevice, 1, &vkInFightFence);
	if (vkQueueSubmit(m_pDevice->m_VkGraphicsQueue, 1, &submitInfo, vkInFightFence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void MVulkanRenderer::NextSubpass(MRenderCommand* pCommand)
{
	if (!pCommand)
		return;

//	vkCmdNextSubpass(rs.vkCommandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	vkCmdNextSubpass(pCommand->m_VkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

	if (!pCommand->m_vRenderPassStages.empty())
	{
		pCommand->m_vRenderPassStages.top().nSubpassIdx++;
	}
}

void MVulkanRenderer::BeginRenderPass(MRenderCommand* pCommand, MRenderPass* pRenderPass, const uint32_t& nFrameBufferIdx)
{
	if (!pCommand || !pRenderPass)
		return;

	if (VK_NULL_HANDLE == pRenderPass->m_VkRenderPass)
	{
		if (!m_pDevice->GenerateRenderPass(pRenderPass))
		{
			MLogManager::GetInstance()->Error("MVulkanRenderer::BeginRenderPass error: Generate rp failed.");
			return;
		}
	}

	MFrameBuffer* pFrameBuffer = pRenderPass->GetFrameBuffer();
	if (!pFrameBuffer)
	{
		MLogManager::GetInstance()->Error("MVulkanRenderer::BeginRenderPass error: fb == nullptr.");
		return;
	}


	if (pFrameBuffer->m_aVkFrameBuffer.empty() && !m_pDevice->GenerateFrameBuffer(pRenderPass))
	{
		MLogManager::GetInstance()->Error("MVulkanRenderer::BeginRenderPass error: fb == nullptr.");
		return;
	}

	size_t unBackNum = pFrameBuffer->vBackTextures.size();
	if (unBackNum != pRenderPass->m_vBackDesc.size())
		return;


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderPass->m_VkRenderPass;
	renderPassInfo.framebuffer = pFrameBuffer->m_aVkFrameBuffer[nFrameBufferIdx];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pFrameBuffer->m_vkExtent2D;

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
	vkCmdBeginRenderPass(pCommand->m_VkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	pCommand->m_vRenderPassStages.push(MRenderPassStage(pRenderPass, 0));
}

void MVulkanRenderer::EndRenderPass(MRenderCommand* pCommand)
{
	if (pCommand->m_vRenderPassStages.empty())
		return;


	pCommand->m_vRenderPassStages.pop();

	//End Render Pass
	vkCmdEndRenderPass(pCommand->m_VkCommandBuffer);

//	vkCmdPipelineBarrier(m_vRenderStages.back().vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,)
}

//����Buffer->��Buffer(����)->��ͼ(����)
void MVulkanRenderer::DrawMesh(MRenderCommand* pCommand, MIMesh* pMesh)
{
	if (!pMesh)
		return;

	DrawMesh(pCommand, pMesh, 0, pMesh->GetIndicesLength(), 0);
}

void MVulkanRenderer::DrawMesh(MRenderCommand* pCommand, MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset)
{
	if (!pCommand || !pMesh)
		return;


	if (pMesh->GetNeedGenerate())
		pMesh->GenerateBuffer(m_pDevice);//�����ʹ����һ��CommandBuffer

	if (pMesh->GetNeedUpload())
		pMesh->UploadBuffer(m_pDevice);

	if (MVertexBuffer* pBuffer = pMesh->GetBuffer())
	{
		VkBuffer vertexBuffers[] = { pBuffer->m_VkVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(pCommand->m_VkCommandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(pCommand->m_VkCommandBuffer, pBuffer->m_VkIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(pCommand->m_VkCommandBuffer, nIdxCount, 1, nIdxOffset, nVrtOffset, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}
}

//��������->�󶨹���(����)->�󶨲��ʲ���(����)
bool MVulkanRenderer::SetUseMaterial(MRenderCommand* pCommand, MMaterial* pMaterial)
{
	if (!pCommand)
		return false;

	//must begin renderpass
	if (pCommand->m_vRenderPassStages.empty())
		return false;

	if (nullptr == pMaterial)
	{
		pCommand->pUsingMaterial = nullptr;
		pCommand->pUsingPipelineLayoutData = nullptr;
		return true;
	}

	MRenderPassStage stage = pCommand->m_vRenderPassStages.top();
	MMaterialPipelineLayoutData* pPipelineLayoutData = m_pDevice->m_PipelineManager.FindOrCreatePipelineLayout(pMaterial);

	pCommand->pUsingMaterial = pMaterial;
	pCommand->pUsingPipelineLayoutData = pPipelineLayoutData;

	VkPipeline vkPipeline = m_pDevice->m_PipelineManager.FindPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx);
	if (!vkPipeline)
	{
		vkPipeline = CreateGraphicsPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx);
		m_pDevice->m_PipelineManager.SetPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx, vkPipeline);
	}
	
	if (vkPipeline)
	{
		vkCmdBindPipeline(pCommand->m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		MShaderParamSet* pParamSet = pMaterial->GetMaterialParamSet();
		SetShaderParamSet(pCommand, pParamSet);

		return true;
	}

	return false;
}

bool MVulkanRenderer::SetRenderToTextureBarrier(MRenderCommand* pCommand, const std::vector<MIRenderTexture*> vTextures)
{
	if (!pCommand)
		return false;

	std::vector<VkImageMemoryBarrier> vImageBarrier;

	for (uint32_t i = 0; i < vTextures.size(); ++i)
	{
		if (MTextureBuffer* pBuffer = vTextures[i]->GetBuffer(pCommand->m_unFrameIdx))
		{
			vImageBarrier.push_back(VkImageMemoryBarrier());
			VkImageMemoryBarrier& imageMemoryBarrier = vImageBarrier.back();

			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = pBuffer->m_VkImageLayout;
			imageMemoryBarrier.newLayout = pBuffer->m_VkImageLayout;
			imageMemoryBarrier.image = pBuffer->m_VkTextureImage;
			imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
	}
	vkCmdPipelineBarrier(
		pCommand->m_VkCommandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		vImageBarrier.size(), vImageBarrier.data());
}

bool MVulkanRenderer::DownloadTexture(MRenderCommand* pCommand, MITexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback)
{
	if (!pCommand ||!pTexture)
		return false;

	MTextureBuffer* pBuffer = pTexture->GetBuffer(pCommand->m_unFrameIdx);
	if (!pBuffer)
		return false;

	uint32_t unValidMipIdx = unMipIdx;
	if (unValidMipIdx >= pBuffer->m_unMipmaps)
		unValidMipIdx = pBuffer->m_unMipmaps - 1;

	VkCommandBuffer commandBuffer = pCommand->m_VkCommandBuffer;
	
	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = pBuffer->m_unMipmaps;
	vkSubresourceRange.layerCount = 1;

	Vector2 size = pTexture->GetSize();
	VkImage textureImage = pBuffer->m_VkTextureImage;

	uint64_t unBufferWidth = size.x;
	uint64_t unBufferHeight = size.y;
	
	for (int i = 0; i < unValidMipIdx; ++i)
	{
		if(unBufferWidth > 1)
			unBufferWidth /= 2;
		if(unBufferHeight > 1)
			unBufferHeight /= 2;
	}

	uint32_t unBufferSize = unBufferWidth * unBufferHeight * static_cast<uint32_t>(MTexture::GetImageMemorySize(pTexture->GetType()));
	

	uint32_t unMemoryID = MGlobal::M_INVALID_INDEX;
	MemoryInfo memoryInfo;
	VkBuffer readBackBuffer = m_pDevice->m_BufferPool.GetReadBackBuffer();
	if (!m_pDevice->m_BufferPool.AllowReadBackBuffer(unBufferSize, unMemoryID, memoryInfo))
	{
		return false;
	}
	
	VkBufferImageCopy region = {};
	region.bufferOffset = memoryInfo.begin;
	region.bufferRowLength = unBufferWidth;
	region.bufferImageHeight = unBufferHeight;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = unValidMipIdx;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageOffset.z = 0;
	region.imageExtent.width = unBufferWidth;
	region.imageExtent.height = unBufferHeight;	// copy to size
	region.imageExtent.depth = 1;



	m_pDevice->TransitionImageLayout(textureImage, pBuffer->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkSubresourceRange, commandBuffer);

	vkCmdCopyImageToBuffer(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readBackBuffer, 1, &region);

	m_pDevice->TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pBuffer->m_VkImageLayout, vkSubresourceRange, commandBuffer);

	pCommand->m_aRenderFinishedCallback.push_back([=]() {

		MByte* data = m_pDevice->m_BufferPool.GetReadBackMemory();
		callback(data + memoryInfo.begin, Vector2(unBufferWidth, unBufferHeight));

		m_pDevice->m_BufferPool.FreeReadBackBuffer(unMemoryID);
		});

}

bool MVulkanRenderer::CopyImageBuffer(MRenderCommand* pCommand, MITexture* pSource, MITexture* pDest)
{
	if (!pCommand)
		return false;

	VkCommandBuffer commandBuffer = pCommand->m_VkCommandBuffer;

	MTextureBuffer* pSourBuffer = pSource->GetBuffer(pCommand->m_unFrameIdx);
	MTextureBuffer* pDestBuffer = pDest->GetBuffer(pCommand->m_unFrameIdx);

	if (!pSourBuffer || !pDestBuffer)
		return false;

	m_pDevice->CopyImageBuffer(pSourBuffer, pDestBuffer, commandBuffer);

	return true;
}

void MVulkanRenderer::UpdateMipmaps(MRenderCommand* pCommand, MTextureBuffer* pBuffer)
{
	if (!pCommand || !pBuffer)
		return;

	VkCommandBuffer commandBuffer = pCommand->m_VkCommandBuffer;

	m_pDevice->GenerateMipmaps(pBuffer, pBuffer->m_unMipmaps, commandBuffer);
	
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
	//	m_pDevice->m_pD3dContext->OMSetDepthStencilState(m_vDepthStencilState[(int)MEDepthStencilType::EReadNotWrite], 0);
		
		if (pRenderPass->m_vBackDesc.size() < 4)
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
	else if (MEMaterialType::EImGui == eType)
	{
		for (uint32_t i = 0; i < pRenderPass->m_vBackDesc.size(); ++i)
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

void MVulkanRenderer::UpdateShaderParam(MShaderConstantParam& param, const uint32_t& unFrameIdx)
{
	if (VK_NULL_HANDLE == param.m_VkBuffer[unFrameIdx])
		return;

	if (param.bDirty && param.m_pMemoryMapping[unFrameIdx])
	{
		memcpy(param.m_pMemoryMapping[unFrameIdx] + param.m_unMemoryOffset[unFrameIdx], param.var.GetData(), param.var.GetSize());

		//TODO android 或其它平台可能需要手动刷新一下缓存
// 		VkMappedMemoryRange memoryRange = {};
// 		memoryRange.sType = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
// 		memoryRange.memory = param.m_VkBufferMemory[pCommand->m_unFrameIdx];
// 		memoryRange.offset = param.m_unMemoryOffset[pCommand->m_unFrameIdx];
// 		memoryRange.size = param.m_unVkMemorySize;
// 		vkFlushMappedMemoryRanges(m_pDevice->m_VkDevice, 1, &memoryRange);

		param.bDirty[unFrameIdx] = false;
	}

}

void MVulkanRenderer::SetShaderParamSet(MRenderCommand* pCommand, MShaderParamSet* pParamSet)
{
	if (!pCommand)
		return;

	MMaterialPipelineLayoutData* pLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
	{
		if (!pCommand->pUsingMaterial)
			return;
		if (!pCommand->pUsingPipelineLayoutData)
			return;

		pParamSet->m_nDescriptorSetInitMaterialIdx = pCommand->pUsingMaterial->GetMaterialID();

		pParamSet->GenerateBuffer(m_pDevice);
	}

	std::vector<uint32_t> vDynamicOffsets;

	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		UpdateShaderParam(*pParam, pCommand->m_unFrameIdx);

		if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			vDynamicOffsets.push_back(pParam->m_unMemoryOffset[pCommand->m_unFrameIdx]);
		}
	}

	for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
	{
		if (pParam->bDirty[pCommand->m_unFrameIdx])
		{
			m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, pParam, pCommand->m_unFrameIdx);
			pParam->bDirty[pCommand->m_unFrameIdx] = false;
		}
		else
		{
			if (pParam->pTexture)
			{
				if (MTextureBuffer* pBuffer = pParam->pTexture->GetBuffer(pCommand->m_unFrameIdx))
				{
					if (pBuffer->m_VkTextureImage != pParam->m_VkUpdatedImage)
					{
						m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, pParam, pCommand->m_unFrameIdx);
					}
				}
			}
		}

		//all else

	}


	vkCmdBindDescriptorSets(pCommand->m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pCommand->pUsingPipelineLayoutData->pipelineLayout, pParamSet->m_unKey, 1, &pParamSet->m_VkDescriptorSet[pCommand->m_unFrameIdx], vDynamicOffsets.size(), vDynamicOffsets.data());
}

VkPipeline MVulkanRenderer::CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx)
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
	pipelineInfo.renderPass = pRenderPass->m_VkRenderPass;
	pipelineInfo.subpass = nSubpassIdx;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return graphicsPipeline;
}


#endif
