#include "MVulkanRenderCommand.h"

#include "MMesh.h"
#include "MMaterial.h"
#include "MVertexBuffer.h"

MVulkanRenderCommand::MVulkanRenderCommand()
	:MIRenderCommand()
{
	m_pDevice = nullptr;

	pUsingMaterial = nullptr;
	pUsingPipelineLayoutData = nullptr;
	m_vRenderPassStages = {};

	m_VkCommandBuffer = VK_NULL_HANDLE;

}

MVulkanRenderCommand::~MVulkanRenderCommand()
{

}

void MVulkanRenderCommand::SetViewport(const MViewportInfo& viewport)
{
	VkViewport vkViewport = {};
	vkViewport.x = viewport.x;
	vkViewport.y = viewport.y + viewport.height;
	vkViewport.width = viewport.width;
	vkViewport.height = -viewport.height;
	vkViewport.minDepth = viewport.minz;
	vkViewport.maxDepth = viewport.maxz;

	vkCmdSetViewport(m_VkCommandBuffer, 0, 1, &vkViewport);
}

void MVulkanRenderCommand::SetScissor(const MScissorInfo& scissor)
{
	VkRect2D scissorRect = { int32_t(scissor.x), int32_t(scissor.y), uint32_t(scissor.width), uint32_t(scissor.height) };
	vkCmdSetScissor(m_VkCommandBuffer, 0, 1, &scissorRect);
}

void MVulkanRenderCommand::RenderCommandBegin()
{
	vkResetCommandBuffer(m_VkCommandBuffer, 0);

	//CommandBuffer Begin Info
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin Command Buffer
	vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo);
}

void MVulkanRenderCommand::RenderCommandEnd()
{
	//End Command Buffer
	vkEndCommandBuffer(m_VkCommandBuffer);
}

void MVulkanRenderCommand::BeginRenderPass(MRenderPass* pRenderPass)
{
	//TODO check renderpass valid.

	SetTextureLayout(pRenderPass->m_vBackTextures, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	
	if (pRenderPass->m_pDepthTexture)
	{
		SetTextureLayout({ pRenderPass->m_pDepthTexture }, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = pRenderPass->m_VkRenderPass;
	renderPassInfo.framebuffer = pRenderPass->m_VkFrameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = pRenderPass->m_vkExtent2D;

	size_t unBackNum = pRenderPass->m_vBackTextures.size();

	std::vector<VkClearValue> vClearValues(unBackNum);
	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MColor color = pRenderPass->m_vBackDesc[i].cClearColor;
		vClearValues[i].color = { color.r, color.g, color.b, color.a };
	}

	if (pRenderPass->m_pDepthTexture)
	{
		vClearValues.push_back({});
		vClearValues.back().depthStencil = { 1.0f, 0 };
	}

	renderPassInfo.clearValueCount = vClearValues.size();
	renderPassInfo.pClearValues = vClearValues.data();

	//Begin RenderPass
	vkCmdBeginRenderPass(m_VkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_vRenderPassStages.push(MRenderPassStage(pRenderPass, 0));
}

void MVulkanRenderCommand::NextSubpass()
{
	if (m_vRenderPassStages.empty())
		return;

	vkCmdNextSubpass(m_VkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
	m_vRenderPassStages.top().nSubpassIdx++;
}

void MVulkanRenderCommand::EndRenderPass()
{
	if (m_vRenderPassStages.empty())
		return;

	m_vRenderPassStages.pop();

	//End Render Pass
	vkCmdEndRenderPass(m_VkCommandBuffer);
}

void MVulkanRenderCommand::DrawMesh(MIMesh* pMesh)
{
	if (!pMesh)
		return;

	DrawMesh(pMesh, 0, pMesh->GetIndicesLength(), 0);
}

void MVulkanRenderCommand::DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset)
{
	if (!pMesh)
		return;

	if (0 == nIdxCount)
		return;

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
		vkCmdDrawIndexed(m_VkCommandBuffer, nIdxCount, 1, nIdxOffset, nVrtOffset, 0);

#if MORTY_RENDER_DATA_STATISTICS
		MRenderStatistics::GetInstance()->unTriangleCount += pMesh->GetIndicesLength() / 3;
#endif
	}
}

bool MVulkanRenderCommand::SetUseMaterial(MMaterial* pMaterial)
{
	//must begin renderpass
	if (m_vRenderPassStages.empty())
		return false;

	if (nullptr == pMaterial)
	{
		pUsingMaterial = nullptr;
		pUsingPipelineLayoutData = nullptr;
		return true;
	}

	MRenderPassStage stage = m_vRenderPassStages.top();
	MMaterialPipelineLayoutData* pPipelineLayoutData = m_pDevice->m_PipelineManager.FindOrCreatePipelineLayout(pMaterial);

	pUsingMaterial = pMaterial;
	pUsingPipelineLayoutData = pPipelineLayoutData;

	VkPipeline vkPipeline = m_pDevice->m_PipelineManager.FindPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx);
	if (!vkPipeline)
	{
		vkPipeline = CreateGraphicsPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx);
		m_pDevice->m_PipelineManager.SetPipeline(pMaterial, stage.pRenderPass, stage.nSubpassIdx, vkPipeline);
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

/*

1. Create Pipeline and PipelineLayout from material. 

2. Life cycle of DescriptorSetLayout depends on Pipeline. DescriptorSetLayout will be destroy when destroy Pipeline.

3. Generate VkDescriptorSet, need DescriptorSetLayout.

4. 

*/

void MVulkanRenderCommand::SetShaderParamSet(MShaderParamSet* pParamSet)
{
	MMaterialPipelineLayoutData* pLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	if (!pLayoutData)
	{
		if (!pUsingMaterial)
			return;
		if (!pUsingPipelineLayoutData)
			return;

		pParamSet->DestroyBuffer(m_pDevice);

		pParamSet->m_nDescriptorSetInitMaterialIdx = pUsingMaterial->GetMaterialID();

		pParamSet->GenerateBuffer(m_pDevice);
	}

	bool bDirty = false;
	std::vector<uint32_t> vDynamicOffsets;
	for (MShaderConstantParam* pParam : pParamSet->m_vParams)
	{
		bDirty |= pParam->bDirty;
		// generate buffer and fill it.
		UpdateShaderParam(pParamSet, pParam);

		if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			vDynamicOffsets.push_back(pParam->m_unMemoryOffset);
		}
	}
	for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
	{
		bDirty |= pParam->bDirty;
		if (pParam->pTexture)
		{
			bDirty |= pParam->pImageIdent != (pParam->pTexture ? pParam->pTexture->m_VkImageView : nullptr);
		}
	}

	if (bDirty)
	{
		//alloc a new descriptor set.
		m_pDevice->m_PipelineManager.AllocateShaderParamSet(pParamSet);

		for (MShaderConstantParam* pParam : pParamSet->m_vParams)
		{
			// bind buffer to descriptor set.
			m_pDevice->m_PipelineManager.BindConstantParam(pParamSet, pParam);
			pParam->bDirty = false;
		}

		for (MShaderTextureParam* pParam : pParamSet->m_vTextures)
		{
			m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, pParam);
			pParam->pImageIdent = pParam->pTexture ? pParam->pTexture->m_VkImageView : nullptr;
			pParam->bDirty = false;
		}
	}

	vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pUsingPipelineLayoutData->pipelineLayout, pParamSet->m_unKey, 1, &pParamSet->m_VkDescriptorSet, vDynamicOffsets.size(), vDynamicOffsets.data());
}

bool MVulkanRenderCommand::SetRenderToTextureBarrier(const std::vector<MTexture*> vTextures)
{
	SetTextureLayout(vTextures, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return true;
}

VkPipelineStageFlags GetSrcPipelineStageFlags(VkImageLayout imageLayout)
{
	switch (imageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	case VK_IMAGE_LAYOUT_UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	default:
		return VK_PIPELINE_STAGE_NONE_KHR;
	}

	return VK_PIPELINE_STAGE_NONE_KHR;
}

VkPipelineStageFlags GetDstPipelineStageFlags(VkImageLayout imageLayout)
{
	switch (imageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	default:
		return VK_PIPELINE_STAGE_NONE_KHR;
	}

	return VK_PIPELINE_STAGE_NONE_KHR;
}

void MVulkanRenderCommand::SetTextureLayout(const std::vector<MTexture*>& vTextures, VkImageLayout newLayout)
{
	std::vector<VkImageMemoryBarrier> vImageBarrier;

	VkPipelineStageFlags srcPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;
	VkPipelineStageFlags dstPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;

	for (uint32_t i = 0; i < vTextures.size(); ++i)
	{
		VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		auto findResult = m_tTextureLayout.find(vTextures[i]);
		if (findResult != m_tTextureLayout.end())
			oldLayout = findResult->second;
		
//		if(oldLayout == newLayout)
//			continue;

		VkImageSubresourceRange subresourceRange;
		if (vTextures[i]->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
		{
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = vTextures[i]->m_unMipmapLevel;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = 1;
		}
		else
		{
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = vTextures[i]->m_unMipmapLevel;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = 1;
		}

		vImageBarrier.push_back(VkImageMemoryBarrier());
		VkImageMemoryBarrier& imageMemoryBarrier = vImageBarrier.back();

		m_pDevice->TransitionImageLayout(imageMemoryBarrier, vTextures[i]->m_VkTextureImage, oldLayout, newLayout, subresourceRange);

		m_tTextureLayout[vTextures[i]] = newLayout;

		srcPipelineStage |= GetSrcPipelineStageFlags(oldLayout);
		dstPipelineStage |= GetDstPipelineStageFlags(newLayout);
	}

	if (vImageBarrier.empty())
		return;

	vkCmdPipelineBarrier(
		m_VkCommandBuffer,
		srcPipelineStage,
		dstPipelineStage,
		0,
		0, nullptr,
		0, nullptr,
		vImageBarrier.size(), vImageBarrier.data());
}

bool MVulkanRenderCommand::DownloadTexture(MTexture* pTexture, const uint32_t& unMipIdx, const std::function<void(void* pImageData, const Vector2& size)>& callback)
{
	if (!pTexture)
		return false;

	uint32_t unValidMipIdx = unMipIdx;
	if (unValidMipIdx >= pTexture->m_unMipmapLevel)
		unValidMipIdx = pTexture->m_unMipmapLevel - 1;

	Vector2 size = pTexture->GetSize();
	VkImage textureImage = pTexture->m_VkTextureImage;

	uint64_t unBufferWidth = size.x;
	uint64_t unBufferHeight = size.y;

	for (int i = 0; i < unValidMipIdx; ++i)
	{
		if (unBufferWidth > 1)
			unBufferWidth /= 2;
		if (unBufferHeight > 1)
			unBufferHeight /= 2;
	}

	uint32_t unBufferSize = unBufferWidth * unBufferHeight * static_cast<uint32_t>(MTexture::GetImageMemorySize(pTexture->GetTextureLayout()));


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



	SetTextureLayout({ pTexture }, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	vkCmdCopyImageToBuffer(m_VkCommandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readBackBuffer, 1, &region);

	m_aRenderFinishedCallback.push_back([=]() {

		MByte* data = m_pDevice->m_BufferPool.GetReadBackMemory();
		callback(data + memoryInfo.begin, Vector2(unBufferWidth, unBufferHeight));

		m_pDevice->m_BufferPool.FreeReadBackBuffer(unMemoryID);
		});

	return true;
}

bool MVulkanRenderCommand::CopyImageBuffer(MTexture* pSource, MTexture* pDest)
{
	if (!pSource || !pDest)
		return false;

	SetTextureLayout({ pSource }, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	SetTextureLayout({ pDest }, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(pSource->GetSize().x), static_cast<int32_t>(pSource->GetSize().y), 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { static_cast<int32_t>(pDest->GetSize().x), static_cast<int32_t>(pDest->GetSize().y), 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	vkCmdBlitImage(m_VkCommandBuffer, pSource->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDest->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);


	return true;
}

void MVulkanRenderCommand::UpdateMipmaps(MTexture* pTexture)
{
	if (!pTexture)
		return;

	m_pDevice->GenerateMipmaps(pTexture, pTexture->m_unMipmapLevel, m_VkCommandBuffer);
}

void MVulkanRenderCommand::UpdateShaderParam(MShaderParamSet* pParamSet, MShaderConstantParam* param)
{
	//if (VK_NULL_HANDLE == param->m_VkBuffer)
	//	return;

	if (param->bDirty)
	{
		m_pDevice->DestroyShaderParamBuffer(param);
		m_pDevice->GenerateShaderParamBuffer(param);

		if (param->m_pMemoryMapping)
		{
			memcpy(param->m_pMemoryMapping + param->m_unMemoryOffset, param->var.GetData(), param->var.GetSize());

//			TODO android 或其它平台可能需要手动刷新一下缓存
// 			VkMappedMemoryRange memoryRange = {};
// 			memoryRange.sType = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
// 			memoryRange.memory = param.m_VkBufferMemory[pCommand->m_unFrameIdx];
// 			memoryRange.offset = param.m_unMemoryOffset[pCommand->m_unFrameIdx];
// 			memoryRange.size = param.m_unVkMemorySize;
// 			vkFlushMappedMemoryRanges(m_pDevice->m_VkDevice, 1, &memoryRange);
		}
	}
}

void MVulkanRenderCommand::UpdateShaderParam(MShaderParamSet* pParamSet, MShaderTextureParam* param)
{
	//texture generate buffer again. its imageIdent will diff.
	if (param->bDirty || (param->pTexture && param->pImageIdent != param->pTexture->m_VkImageView))
	{
		m_pDevice->m_PipelineManager.BindTextureParam(pParamSet, param);
		param->bDirty = false;
		param->pImageIdent = param->pTexture ? param->pTexture->m_VkImageView : nullptr;
	}
}

void GetBlendStage(MMaterial* pMaterial, MRenderPass* pRenderPass, std::vector<VkPipelineColorBlendAttachmentState>& vBlendAttach, VkPipelineColorBlendStateCreateInfo& blendInfo)
{


	blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.logicOp = VK_LOGIC_OP_COPY;

	MEMaterialType eType = pMaterial->GetMaterialType();

	if (MEMaterialType::EDefault == eType || MEMaterialType::EDeferred == eType)
	{
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

void GetDepthStencilStage(MMaterial* pMaterial, MRenderPass* pRenderPass, VkPipelineDepthStencilStateCreateInfo& depthStencilInfo)
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

VkPipeline MVulkanRenderCommand::CreateGraphicsPipeline(MMaterial* pMaterial, MRenderPass* pRenderPass, const uint32_t& nSubpassIdx)
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
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = pRenderPass->m_VkRenderPass;
	pipelineInfo.subpass = nSubpassIdx;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(m_pDevice->m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return graphicsPipeline;
}

void MVulkanRenderCommand::BindConstantParam(MShaderParamSet* pParamSet, MShaderConstantParam* pParam)
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

	//vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);

	MMaterialPipelineLayoutData* pLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
	m_pDevice->vkCmdPushDescriptorSet(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pLayoutData->pipelineLayout, pParam->unSet, 1, &descriptorWrite);
}

void MVulkanRenderCommand::BindTextureParam(MShaderParamSet* pParamSet, MShaderTextureParam* pParam)
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
		//vkUpdateDescriptorSets(m_pDevice->m_VkDevice, 1, &descriptorWrite, 0, nullptr);

		MMaterialPipelineLayoutData* pLayoutData = m_pDevice->m_PipelineManager.FindPipelineLayout(pParamSet->m_nDescriptorSetInitMaterialIdx);
		m_pDevice->vkCmdPushDescriptorSet(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pLayoutData->pipelineLayout, pParam->unSet, 1, &descriptorWrite);
	}
}

MVulkanPrimaryRenderCommand::MVulkanPrimaryRenderCommand()
	: MVulkanRenderCommand()
{
	m_VkRenderFinishedFence = VK_NULL_HANDLE;
	m_VkRenderFinishedSemaphore = VK_NULL_HANDLE;

	m_vRenderWaitSemaphore = {};

	m_bFinished = false;
}

void MVulkanPrimaryRenderCommand::CheckFinished()
{
	if (m_bFinished) return;

	if (m_pDevice->IsFinishedCommand(this))
	{
		m_bFinished = true;

		for (auto& callback : m_aRenderFinishedCallback)
		{
			callback();
		}

		m_aRenderFinishedCallback.clear();
	}
}

MIRenderCommand* MVulkanPrimaryRenderCommand::CreateChildCommand()
{
	MVulkanSecondaryRenderCommand* pChildCommand = m_pDevice->CreateChildCommand(this);
	m_vSecondaryCommand.push_back(pChildCommand);

	return pChildCommand;
}

MIRenderCommand* MVulkanPrimaryRenderCommand::GetChildCommand(const size_t& nIndex)
{
	if(nIndex < m_vSecondaryCommand.size())
		return m_vSecondaryCommand[nIndex];
	return nullptr;
}

void MVulkanPrimaryRenderCommand::ExecuteChildCommand()
{
	std::vector<VkCommandBuffer> buffers;
	for (MVulkanSecondaryRenderCommand* pChildCommand : m_vSecondaryCommand)
		buffers.push_back(pChildCommand->m_VkCommandBuffer);

	vkCmdExecuteCommands(m_VkCommandBuffer, buffers.size(), buffers.data());
}
