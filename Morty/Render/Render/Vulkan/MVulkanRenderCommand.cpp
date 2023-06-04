#include "Render/Vulkan/MVulkanRenderCommand.h"

#include "Render/MMesh.h"
#include "Render/MVertexBuffer.h"

#include "Material/MMaterial.h"
#include "Material/MComputeDispatcher.h"

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

	m_nDrawCallCount = 0;

	pUsingPipeline = nullptr;
}

void MVulkanRenderCommand::RenderCommandEnd()
{
	//End Command Buffer
	vkEndCommandBuffer(m_VkCommandBuffer);
}

void MVulkanRenderCommand::BeginRenderPass(MRenderPass* pRenderPass)
{
	//TODO check renderpass valid.

	std::vector<MTexture*> vTextures;
	for (auto pTexture : pRenderPass->GetBackTextures())
	{
		vTextures.push_back(pTexture.get());
	}

	SetTextureLayout(vTextures, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	
	if (std::shared_ptr<MTexture> pDepthTexture = pRenderPass->GetDepthTexture())
	{
		SetTextureLayout({ pDepthTexture.get() }, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
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
		MColor color = pRenderPass->m_vBackTextures[i].desc.cClearColor;
		vClearValues[i].color = { color.r, color.g, color.b, color.a };
	}

	if (std::shared_ptr<MTexture> pTexture = pRenderPass->GetDepthTexture())
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

	DrawMesh(pMesh, 0, pMesh->GetIndicesNum(), 0);
}

void MVulkanRenderCommand::DrawMesh(MIMesh* pMesh, const uint32_t& nIdxOffset, const uint32_t& nIdxCount, const uint32_t& nVrtOffset)
{
	if (!pMesh)
		return;

	if (0 == nIdxCount)
		return;

	MBuffer* pVertexBuffer = pMesh->GetVertexBuffer();
	MBuffer* pIndexBuffer = pMesh->GetIndexBuffer();

	if (!pVertexBuffer || !pIndexBuffer)
	{
		return;
	}

	UpdateBuffer(pVertexBuffer, pMesh->GetVerticesVector().data(), pMesh->GetVerticesVector().size());
	UpdateBuffer(pIndexBuffer, pMesh->GetIndicesVector().data(), pMesh->GetIndicesVector().size());

	DrawMesh(pVertexBuffer, pIndexBuffer, nVrtOffset, nIdxOffset, nIdxCount);
}

void MVulkanRenderCommand::DrawMesh(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const size_t nVertexOffset, const size_t nIndexOffset, const size_t nIndexCount)
{
	if (!pVertexBuffer || !pIndexBuffer)
	{
		MORTY_ASSERT(pVertexBuffer && pIndexBuffer);
		return;
	}

	if (pVertexBuffer->m_VkBuffer == VK_NULL_HANDLE || pIndexBuffer->m_VkBuffer == VK_NULL_HANDLE)
	{
		MORTY_ASSERT(pVertexBuffer->m_VkBuffer != VK_NULL_HANDLE && pIndexBuffer->m_VkBuffer != VK_NULL_HANDLE);
		return;
	}

	if (pUsingVertex != pVertexBuffer)
	{
		const VkBuffer vertexBuffers[] = { pVertexBuffer->m_VkBuffer };
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_VkCommandBuffer, 0, 1, vertexBuffers, offsets);
		pUsingVertex = pVertexBuffer;
	}

	if (pUsingIndex != pIndexBuffer)
	{
		vkCmdBindIndexBuffer(m_VkCommandBuffer, pIndexBuffer->m_VkBuffer, 0, VK_INDEX_TYPE_UINT32);
		pUsingIndex = pIndexBuffer;
	}

	vkCmdDrawIndexed(m_VkCommandBuffer, nIndexCount, 1, nIndexOffset, nVertexOffset, 0);

	++m_nDrawCallCount;
}

void MVulkanRenderCommand::DrawIndexedIndirect(const MBuffer* pVertexBuffer, const MBuffer* pIndexBuffer, const MBuffer* pCommandsBuffer, const size_t& offset, const size_t& count)
{
	if (pUsingVertex != pVertexBuffer)
	{
		const VkBuffer vertexBuffers[] = { pVertexBuffer->m_VkBuffer };
		constexpr VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_VkCommandBuffer, 0, 1, vertexBuffers, offsets);
		pUsingVertex = pVertexBuffer;
	}

	if (pUsingIndex != pIndexBuffer)
	{
		vkCmdBindIndexBuffer(m_VkCommandBuffer, pIndexBuffer->m_VkBuffer, 0, VK_INDEX_TYPE_UINT32);
		pUsingIndex = pIndexBuffer;
	}

	vkCmdDrawIndexedIndirect(m_VkCommandBuffer, pCommandsBuffer->m_VkBuffer, offset, count, sizeof(VkDrawIndexedIndirectCommand));

	++m_nDrawCallCount;
}

bool MVulkanRenderCommand::SetUseMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	MORTY_ASSERT(pMaterial->GetVertexShader() && pMaterial->GetPixelShader());

	//must begin renderpass
	if (m_vRenderPassStages.empty())
	{
		MORTY_ASSERT(!m_vRenderPassStages.empty());
		return false;
	}

	if (nullptr == pMaterial)
	{
		MORTY_ASSERT(nullptr != pMaterial);
		pUsingPipeline = nullptr;
		return true;
	}

	pUsingVertex = nullptr;
	pUsingIndex = nullptr;

	MRenderPassStage stage = m_vRenderPassStages.top();
	std::shared_ptr<MPipeline> pPipeline = m_pDevice->m_PipelineManager.FindOrCreateGraphicsPipeline(pMaterial, stage.pRenderPass);
	MORTY_ASSERT(pUsingPipeline = pPipeline);

	if (std::shared_ptr<MGraphicsPipeline> pGraphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pPipeline))
	{
		VkPipeline vkPipeline = pGraphicsPipeline->GetSubpassPipeline(stage.nSubpassIdx);
		MORTY_ASSERT(vkPipeline != VK_NULL_HANDLE);

		vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		std::shared_ptr<MShaderPropertyBlock> pParamSet = pMaterial->GetMaterialParamSet();

		SetShaderParamSet(pParamSet);

		return true;
	}

	return false;
}

bool MVulkanRenderCommand::DispatchComputeJob(MComputeDispatcher* pComputeDispatcher, const uint32_t& nGroupX, const uint32_t& nGroupY, const uint32_t& nGroupZ)
{
	MORTY_ASSERT(pComputeDispatcher->GetComputeShader());

	if (nullptr == pComputeDispatcher)
	{
		return true;
	}

	std::shared_ptr<MPipeline> pPipeline = m_pDevice->m_PipelineManager.FindOrCreateComputePipeline(pComputeDispatcher);
	MORTY_ASSERT(pUsingPipeline = pPipeline);

	if (std::shared_ptr<MComputePipeline> pComputePipeline = std::dynamic_pointer_cast<MComputePipeline>(pPipeline))
	{
		VkPipeline vkPipeline = pComputePipeline->m_vkPipeline;
		MORTY_ASSERT(VK_NULL_HANDLE != vkPipeline);

		vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);

		for (const std::shared_ptr<MShaderPropertyBlock>& params : pComputeDispatcher->GetShaderParamSets())
		{
			SetShaderParamSet(params);
		}

		vkCmdDispatch(m_VkCommandBuffer, nGroupX, nGroupY, nGroupZ);

		return true;
	}

	return false;
}

void MVulkanRenderCommand::SetShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
	std::shared_ptr<MShaderProgram> pShaderPropgram = pPropertyBlock->GetShaderProgram();
	MORTY_ASSERT(pShaderPropgram);

	bool bDirty = false;
	for (std::shared_ptr<MShaderConstantParam> pParam : pPropertyBlock->m_vParams)
	{
		if (pParam->bDirty)
		{
			bDirty = true;
			// generate buffer and fill data.
			UpdateShaderParam(pParam);
			pParam->bDirty = false;
		}

	}
	for (std::shared_ptr<MShaderTextureParam> pParam : pPropertyBlock->m_vTextures)
	{
		auto pImageIdent = pParam->GetTexture() ? pParam->GetTexture()->m_VkImageView : nullptr;
		if (pParam->bDirty || pParam->pImageIdent != pImageIdent)
		{
			bDirty = true;
			pParam->bDirty = false;
			pParam->pImageIdent = pImageIdent;
		}
	}

	for (std::shared_ptr<MShaderStorageParam> pParam : pPropertyBlock->m_vStorages)
	{
		bDirty |= pParam->bDirty;
		pParam->bDirty = false;
	}

	if (bDirty)
	{
		//alloc a new descriptor set.
		m_pDevice->m_PipelineManager.AllocateShaderParamSet(pPropertyBlock, pUsingPipeline);

		std::vector<VkWriteDescriptorSet> vWriteDescriptorSet;

		for (std::shared_ptr<MShaderConstantParam> pParam : pPropertyBlock->m_vParams)
		{
			// bind buffer to descriptor set.
			vWriteDescriptorSet.push_back({});
			VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
			m_pDevice->m_PipelineManager.BindConstantParam(pParam, writeDescriptorSet);
			writeDescriptorSet.dstSet = pPropertyBlock->m_VkDescriptorSet;
		}

		for (std::shared_ptr<MShaderTextureParam> pParam : pPropertyBlock->m_vTextures)
		{
			vWriteDescriptorSet.push_back({});
			VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
			m_pDevice->m_PipelineManager.BindTextureParam(pParam, writeDescriptorSet);
			writeDescriptorSet.dstSet = pPropertyBlock->m_VkDescriptorSet;
		}

		for (std::shared_ptr<MShaderStorageParam> pParam : pPropertyBlock->m_vStorages)
		{
			vWriteDescriptorSet.push_back({});
			VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
			m_pDevice->m_PipelineManager.BindStorageParam(pParam, writeDescriptorSet);
			writeDescriptorSet.dstSet = pPropertyBlock->m_VkDescriptorSet;
		}

		vkUpdateDescriptorSets(m_pDevice->m_VkDevice, vWriteDescriptorSet.size(), vWriteDescriptorSet.data(), 0, nullptr);
	}
	else if (VK_NULL_HANDLE == pPropertyBlock->m_VkDescriptorSet)
	{
		//alloc a new descriptor set.
		m_pDevice->m_PipelineManager.AllocateShaderParamSet(pPropertyBlock, pUsingPipeline);

	}

	MORTY_ASSERT(VK_NULL_HANDLE != pUsingPipeline->m_pipelineLayout.vkPipelineLayout);
	MORTY_ASSERT(VK_NULL_HANDLE != pPropertyBlock->m_VkDescriptorSet);

	std::vector<uint32_t> vDynamicOffsets;
	for (std::shared_ptr<MShaderConstantParam> pParam : pPropertyBlock->m_vParams)
	{
		if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			vDynamicOffsets.push_back(pParam->m_unMemoryOffset);
		}
	}

	VkPipelineBindPoint vkPipelineBindPoint = pUsingPipeline->m_vkPipelineBindPoint;
	vkCmdBindDescriptorSets(m_VkCommandBuffer, vkPipelineBindPoint, pUsingPipeline->m_pipelineLayout.vkPipelineLayout, pPropertyBlock->m_unKey, 1, &pPropertyBlock->m_VkDescriptorSet, vDynamicOffsets.size(), vDynamicOffsets.data());
}

bool MVulkanRenderCommand::AddRenderToTextureBarrier(const std::vector<MTexture*> vTextures)
{
	SetTextureLayout(vTextures, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	return true;
}

bool MVulkanRenderCommand::AddComputeToGraphBarrier(const std::vector<const MBuffer*> vBuffers)
{

	std::vector<VkBufferMemoryBarrier> bufferBarriers;
	for(const MBuffer* pBuffer : vBuffers)
	{
		VkBufferMemoryBarrier bufferBarrier;
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.pNext = nullptr;
		bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		bufferBarrier.srcQueueFamilyIndex = m_pDevice->m_nComputeFamilyIndex;
		bufferBarrier.dstQueueFamilyIndex = m_pDevice->m_nGraphicsFamilyIndex;
		bufferBarrier.buffer = pBuffer->m_VkBuffer;
		bufferBarrier.offset = 0;
		bufferBarrier.size = VK_WHOLE_SIZE;
		bufferBarriers.push_back(bufferBarrier);
	}

	vkCmdPipelineBarrier(
		m_VkCommandBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		0,
		0, nullptr,
		static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
		0, nullptr);


	return true;
}

bool MVulkanRenderCommand::AddGraphToComputeBarrier(const std::vector<const MBuffer*> vBuffers)
{
	std::vector<VkBufferMemoryBarrier> bufferBarriers;
	for (const MBuffer* pBuffer : vBuffers)
	{
		VkBufferMemoryBarrier bufferBarrier;
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.pNext = nullptr;
		bufferBarrier.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		bufferBarrier.srcQueueFamilyIndex = m_pDevice->m_nGraphicsFamilyIndex;
		bufferBarrier.dstQueueFamilyIndex = m_pDevice->m_nComputeFamilyIndex;
		bufferBarrier.buffer = pBuffer->m_VkBuffer;
		bufferBarrier.offset = 0;
		bufferBarrier.size = VK_WHOLE_SIZE;
		bufferBarriers.push_back(bufferBarrier);
	}

	vkCmdPipelineBarrier(
		m_VkCommandBuffer,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
		0, nullptr);


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
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

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
		
		if(oldLayout == newLayout)
			continue;

		VkImageSubresourceRange subresourceRange;
		if (vTextures[i]->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
		{
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = vTextures[i]->m_unMipmapLevel;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = vTextures[i]->GetImageLayerNum();
		}
		else
		{
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = vTextures[i]->m_unMipmapLevel;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = vTextures[i]->GetImageLayerNum();
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
	region.imageSubresource.layerCount = pTexture->GetImageLayerNum();
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
	blit.srcSubresource.layerCount = pSource->GetImageLayerNum();
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { static_cast<int32_t>(pDest->GetSize().x), static_cast<int32_t>(pDest->GetSize().y), 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = pDest->GetImageLayerNum();

	vkCmdBlitImage(m_VkCommandBuffer, pSource->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDest->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);


	return true;
}

void MVulkanRenderCommand::UpdateMipmaps(MTexture* pTexture)
{
	if (!pTexture)
		return;

	m_pDevice->GenerateMipmaps(pTexture, pTexture->m_unMipmapLevel, m_VkCommandBuffer);
}

void MVulkanRenderCommand::addFinishedCallback(std::function<void()> func)
{
	m_aRenderFinishedCallback.push_back(func);
}

void MVulkanRenderCommand::UpdateBuffer(MBuffer* pBuffer, const MByte* data, const size_t& size)
{
	if (!pBuffer)
	{
		return;
	}

	if (pBuffer->m_eStageType == MBuffer::MStageType::EWaitAllow)
	{
		pBuffer->DestroyBuffer(m_pDevice);
		pBuffer->GenerateBuffer(m_pDevice, data, size);
	}
	else if (pBuffer->m_eStageType == MBuffer::MStageType::EWaitSync)
	{
		pBuffer->UploadBuffer(m_pDevice, data, size);
	}
}

void MVulkanRenderCommand::UpdateShaderParam(std::shared_ptr<MShaderConstantParam> param)
{
	m_pDevice->DestroyShaderParamBuffer(param);
	m_pDevice->GenerateShaderParamBuffer(param);

	MORTY_ASSERT(param->m_pMemoryMapping);

	if (param->m_pMemoryMapping)
	{
		memcpy(param->m_pMemoryMapping + param->m_unMemoryOffset, param->var.GetData(), param->var.GetSize());

#ifndef MORTY_WIN
 		VkMappedMemoryRange memoryRange = {};
 		memoryRange.sType = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
 		memoryRange.memory = param->m_VkBufferMemory;
 		memoryRange.offset = param->m_unMemoryOffset;
 		memoryRange.size = param->m_unVkMemorySize;
 		vkFlushMappedMemoryRanges(m_pDevice->m_VkDevice, 1, &memoryRange);
#endif
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
