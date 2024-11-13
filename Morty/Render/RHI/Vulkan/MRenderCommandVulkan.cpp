#include "MRenderCommandVulkan.h"
#include "MVulkanCommandExecuteTable.h"
#include "MVulkanPhysicalDevice.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertexBuffer.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/Vulkan/MTextureRHIVulkan.h"

using namespace morty;

MVulkanCommandExecuteTable MRenderCommandVulkan::m_executeTable = MVulkanCommandExecuteTable();


void                       MRenderCommandVulkan::SetViewport(const MSetViewportCmd* viewport) const
{
    VkViewport vkViewport = {};
    vkViewport.x          = viewport->x;
    vkViewport.y          = viewport->y + viewport->height;
    vkViewport.width      = std::max(viewport->width, 1.0f);
    vkViewport.height     = -std::max(viewport->height, 1.0f);
    vkViewport.minDepth   = viewport->minDepth;
    vkViewport.maxDepth   = viewport->maxDepth;

    vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &vkViewport);
}

void MRenderCommandVulkan::SetScissor(const MSetScissorCmd* scissor) const
{
    VkRect2D scissorRect = {
            VkOffset2D{int32_t(scissor->x), int32_t(scissor->y)},
            VkExtent2D{uint32_t(std::max(scissor->width, 1.0f)), uint32_t(std::max(scissor->height, 1.0f))}
    };

    vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &scissorRect);
}

void MRenderCommandVulkan::DrawMesh(const MDrawMeshCmd* cmd)
{
    MORTY_ASSERT(cmd->vertexBuffer && cmd->indexBuffer);
    MORTY_ASSERT(cmd->vertexBuffer->m_vkBuffer != VK_NULL_HANDLE && cmd->indexBuffer->m_vkBuffer != VK_NULL_HANDLE);

    if (pUsingVertex != cmd->vertexBuffer)
    {
        const VkBuffer         vertexBuffers[] = {cmd->vertexBuffer->m_vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = cmd->vertexBuffer;
    }

    if (pUsingIndex != cmd->indexBuffer)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, cmd->indexBuffer->m_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = cmd->indexBuffer;
    }

    vkCmdDrawIndexed(m_vkCommandBuffer, cmd->indexCount, 1, cmd->indexOffset, cmd->vertexOffset, 0);

    ++m_drawCallCount;
}

void MRenderCommandVulkan::DrawIndexedIndirect(const MDrawIndexedIndirectCmd* cmd)
{
    if (pUsingVertex != cmd->vertexBuffer)
    {
        const VkBuffer         vertexBuffers[] = {cmd->vertexBuffer->m_vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = cmd->vertexBuffer;
    }

    if (pUsingIndex != cmd->indexBuffer)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, cmd->indexBuffer->m_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = cmd->indexBuffer;
    }

    if (m_device->MultiDrawIndirectSupport())
    {
        vkCmdDrawIndexedIndirect(
                m_vkCommandBuffer,
                cmd->commandsBuffer->m_vkBuffer,
                cmd->offset,
                static_cast<uint32_t>(cmd->count),
                sizeof(VkDrawIndexedIndirectCommand)
        );
    }
    else
    {
        for (size_t nDrawIdx = 0; nDrawIdx < cmd->count; ++nDrawIdx)
        {
            vkCmdDrawIndexedIndirect(
                    m_vkCommandBuffer,
                    cmd->commandsBuffer->m_vkBuffer,
                    cmd->offset + sizeof(VkDrawIndexedIndirectCommand) * nDrawIdx,
                    1,
                    sizeof(VkDrawIndexedIndirectCommand)
            );
        }
    }
    ++m_drawCallCount;
}

void MRenderCommandVulkan::SetGraphPipeline(const MSetGraphPipelineCmd* cmd)
{
    VkPipeline vkPipeline = cmd->pipeline->GetSubpassPipeline(cmd->subPassIdx);
    MORTY_ASSERT(vkPipeline);

    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

    pUsingVertex = nullptr;
    pUsingIndex  = nullptr;
}

void MRenderCommandVulkan::SetShaderPropertyBlock(const MSetShaderPropertyBlockCmd* cmd)
{
    auto pPropertyBlock = cmd->property;
    auto pPipeline      = cmd->pipeline;

    bool bNeedAllocDescriptorSet = false;
    for (const auto& pParam: pPropertyBlock->m_params)
    {
        if (pParam->bDirty)
        {
            UpdateShaderParam(pParam);
            pParam->bDirty = false;
        }
    }

    for (const auto& pParam: pPropertyBlock->m_textures)
    {
        const auto pImageIdent =
                pParam->GetTexture() ? pParam->GetTexture()->GetTextureRHI<MTextureRHIVulkan>()->vkImageView : nullptr;
        if (pParam->bDirty || pParam->pImageIdent != pImageIdent)
        {
            bNeedAllocDescriptorSet = true;
            pParam->bDirty          = false;
            pParam->pImageIdent     = pImageIdent;
        }
    }

    for (const auto& pParam: pPropertyBlock->m_storages)
    {
        const auto pStoreIdent = pParam->pBuffer->m_vkBuffer;
        if (pParam->pImageIdent != pStoreIdent)
        {
            bNeedAllocDescriptorSet = true;
            pParam->bDirty          = false;
            pParam->pImageIdent     = pStoreIdent;
        }
    }

    if (VK_NULL_HANDLE == pPropertyBlock->m_vkDescriptorSet) { bNeedAllocDescriptorSet = true; }

    if (bNeedAllocDescriptorSet)
    {
        //alloc a new descriptor set.
        m_device->m_PipelineManager.AllocateShaderPropertyBlock(pPropertyBlock, pPipeline);

        std::vector<VkWriteDescriptorSet> vWriteDescriptorSet;

        for (const auto& pParam: pPropertyBlock->m_params)
        {
            // bind buffer to descriptor set.
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindConstantParam(pParam, writeDescriptorSet);
            writeDescriptorSet.dstSet = pPropertyBlock->m_vkDescriptorSet;
        }

        for (const auto& pParam: pPropertyBlock->m_textures)
        {
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindTextureParam(pParam, writeDescriptorSet);
            writeDescriptorSet.dstSet = pPropertyBlock->m_vkDescriptorSet;
        }

        for (const auto& pParam: pPropertyBlock->m_storages)
        {
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindStorageParam(pParam, writeDescriptorSet);
            writeDescriptorSet.dstSet = pPropertyBlock->m_vkDescriptorSet;
        }

        vkUpdateDescriptorSets(
                m_device->m_vkDevice,
                static_cast<uint32_t>(vWriteDescriptorSet.size()),
                vWriteDescriptorSet.data(),
                0,
                nullptr
        );
    }

    MORTY_ASSERT(VK_NULL_HANDLE != pPipeline->m_pipelineLayout.vkPipelineLayout);
    MORTY_ASSERT(VK_NULL_HANDLE != pPropertyBlock->m_vkDescriptorSet);

    std::vector<uint32_t> vDynamicOffsets;
    for (const auto& pParam: pPropertyBlock->m_params)
    {
        if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        {
            vDynamicOffsets.push_back(pParam->m_unMemoryOffset);
        }
    }

    VkPipelineBindPoint vkPipelineBindPoint = pPipeline->m_vkPipelineBindPoint;
    vkCmdBindDescriptorSets(
            m_vkCommandBuffer,
            vkPipelineBindPoint,
            pPipeline->m_pipelineLayout.vkPipelineLayout,
            pPropertyBlock->m_unKey,
            1,
            &pPropertyBlock->m_vkDescriptorSet,
            static_cast<uint32_t>(vDynamicOffsets.size()),
            vDynamicOffsets.data()
    );
}

void MRenderCommandVulkan::AddBarrierForPixelSample(const MSetShaderPropertyBlockCmd* cmd)
{
    std::vector<MTexture*> vTextures;
    for (const auto& pParam: cmd->property->m_textures)
    {
        if (auto pTexture = pParam->GetTexture().get()) { vTextures.emplace_back(pTexture); }
    }
    AddRenderToTextureBarrier(vTextures, METextureBarrierStage::EPixelShaderSample);
}

void MRenderCommandVulkan::NextSubPass(const MNextSubPassCmd* cmd)
{
    MORTY_UNUSED(cmd);
    vkCmdNextSubpass(m_vkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void MRenderCommandVulkan::SetShadingRate(const MSetShadingRateCmd* cmd)
{
    const VkExtent2D vkShadingSize = {
            static_cast<uint32_t>(cmd->shadingRate.x),
            static_cast<uint32_t>(cmd->shadingRate.y)
    };
    const VkFragmentShadingRateCombinerOpKHR vkCombinerOp[2] = {
            m_device->GetShadingRateCombinerOp(cmd->combineOp[0]),
            m_device->GetShadingRateCombinerOp(cmd->combineOp[1])
    };

    m_device->GetPhysicalDevice()->vkCmdSetFragmentShadingRateKHR(m_vkCommandBuffer, &vkShadingSize, vkCombinerOp);
}

void MRenderCommandVulkan::RenderCommandBegin()
{
    vkResetCommandBuffer(m_vkCommandBuffer, 0);

    //CommandBuffer Begin Info
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    //Begin Command Buffer
    vkBeginCommandBuffer(m_vkCommandBuffer, &beginInfo);

    m_drawCallCount = 0;
}

void MRenderCommandVulkan::RenderCommandEnd()
{
    //End Command Buffer
    vkEndCommandBuffer(m_vkCommandBuffer);
}

void MRenderCommandVulkan::InternalBeginRenderPass(MRenderPass* pRenderPass)
{
    //TODO check renderpass valid.
    if (VK_NULL_HANDLE == pRenderPass->m_vkFrameBuffer) { m_device->GenerateFrameBuffer(pRenderPass); }

    std::vector<MTexture*> vTextures(pRenderPass->GetBackTextures().size());
    for (size_t nIdx = 0; nIdx < pRenderPass->GetBackTextures().size(); ++nIdx)
    {
        vTextures[nIdx] = pRenderPass->GetBackTexture(nIdx).get();
    }
    std::vector<VkImageLayout> vLayouts(
            pRenderPass->GetBackTextures().size(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    );

    SetTextureLayout(vTextures, vLayouts);

    if (MTexturePtr pDepthTexture = pRenderPass->GetDepthTexture())
    {
        SetTextureLayout({pDepthTexture.get()}, {m_device->m_physicalDevice->m_vkDepthImageLayout});
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = pRenderPass->m_vkRenderPass;
    renderPassInfo.framebuffer       = pRenderPass->m_vkFrameBuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = pRenderPass->m_vkExtent2D;

    size_t                    unBackNum = pRenderPass->m_renderTarget.backTargets.size();

    std::vector<VkClearValue> vClearValues(unBackNum);
    std::vector<MTexture*>    vBackTextures(unBackNum);
    for (uint32_t i = 0; i < unBackNum; ++i)
    {
        const MColor color = pRenderPass->m_renderTarget.backTargets[i].desc.cClearColor;
        //-Wmissing-braces
        vClearValues[i].color = {{color.r, color.g, color.b, color.a}};
        vBackTextures[i]      = pRenderPass->m_renderTarget.backTargets[i].pTexture.get();
    }
    AddRenderToTextureBarrier(vBackTextures, METextureBarrierStage::EPixelShaderWrite);


    if (MTexturePtr pTexture = pRenderPass->GetDepthTexture())
    {
        vClearValues.push_back({});
        vClearValues.back().depthStencil = {1.0f, 0};

        AddRenderToTextureBarrier({pTexture.get()}, METextureBarrierStage::EPixelShaderWrite);
    }

    if (auto pShadingRateTex = pRenderPass->GetShadingRateTexture())
    {
        const MColor color = pRenderPass->m_renderTarget.shadingRate.desc.cClearColor;
        vClearValues.push_back({});
        vClearValues.back().color = {{color.r, color.g, color.b, color.a}};

        AddRenderToTextureBarrier({pShadingRateTex.get()}, METextureBarrierStage::EShadingRateMask);
    }

    renderPassInfo.clearValueCount = static_cast<uint32_t>(vClearValues.size());
    renderPassInfo.pClearValues    = vClearValues.data();

    //Begin RenderPass
    vkCmdBeginRenderPass(m_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void MRenderCommandVulkan::InternalEndRenderPass()
{
    //End Render Pass
    vkCmdEndRenderPass(m_vkCommandBuffer);
}

/*
void MRenderCommandVulkan::DrawMesh(
        MIMesh*         pMesh,
        const uint32_t& nIdxOffset,
        const uint32_t& nIdxCount,
        const uint32_t& nVrtOffset
)
{
    if (!pMesh) return;

    if (0 == nIdxCount) return;

    MBuffer* pVertexBuffer = pMesh->GetVertexBuffer();
    MBuffer* pIndexBuffer  = pMesh->GetIndexBuffer();

    if (!pVertexBuffer || !pIndexBuffer) { return; }

    UpdateBuffer(pVertexBuffer, pMesh->GetVerticesVector().data(), pMesh->GetVerticesVector().size());
    UpdateBuffer(pIndexBuffer, pMesh->GetIndicesVector().data(), pMesh->GetIndicesVector().size());

    DrawMesh(pVertexBuffer, pIndexBuffer, nVrtOffset, nIdxOffset, nIdxCount);
}
 */

bool MRenderCommandVulkan::DispatchComputeJob(
        MComputeDispatcher* pComputeDispatcher,
        const uint32_t&     nGroupX,
        const uint32_t&     nGroupY,
        const uint32_t&     nGroupZ
)
{
    if (nullptr == pComputeDispatcher)
    {
        MORTY_ASSERT(pComputeDispatcher);
        return true;
    }

    MORTY_ASSERT(pComputeDispatcher->GetComputeShader());

    std::shared_ptr<MPipeline> pPipeline = m_device->m_PipelineManager.FindOrCreateComputePipeline(pComputeDispatcher);
    MORTY_ASSERT(pPipeline);

    if (std::shared_ptr<MComputePipeline> pComputePipeline = std::dynamic_pointer_cast<MComputePipeline>(pPipeline))
    {
        VkPipeline vkPipeline = pComputePipeline->m_vkPipeline;
        MORTY_ASSERT(VK_NULL_HANDLE != vkPipeline);

        vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);

        for (const std::shared_ptr<MShaderPropertyBlock>& params: pComputeDispatcher->GetShaderPropertyBlocks())
        {
            MSetShaderPropertyBlockCmd cmd{
                    .pipeline = pComputePipeline.get(),
                    .property = params.get(),
            };

            SetShaderPropertyBlock(&cmd);
        }

        vkCmdDispatch(m_vkCommandBuffer, nGroupX, nGroupY, nGroupZ);

        return true;
    }

    return false;
}

bool MRenderCommandVulkan::AddRenderToTextureBarrier(
        const std::vector<MTexture*>& vTextures,
        METextureBarrierStage         dstStage
)
{
    if (vTextures.empty()) { return false; }

    std::vector<VkImageLayout> layouts(vTextures.size());
    std::transform(vTextures.begin(), vTextures.end(), layouts.begin(), [this, dstStage](auto pTexture) {
        return GetTextureBarrierLayout(pTexture, dstStage);
    });


    SetTextureLayout(vTextures, layouts);
    return true;
}

bool MRenderCommandVulkan::AddBufferMemoryBarrier(
        const std::vector<const MBuffer*>& vBuffers,
        MEBufferBarrierStage               srcStage,
        MEBufferBarrierStage               dstStage
)
{
    const auto                         srcAccessMask       = GetBufferBarrierAccessFlag(srcStage);
    const auto                         dstAccessMask       = GetBufferBarrierAccessFlag(dstStage);
    const uint32_t                     srcQueueFamilyIndex = m_device->GetBufferBarrierQueueFamily(srcStage);
    const uint32_t                     dstQueueFamilyIndex = m_device->GetBufferBarrierQueueFamily(dstStage);
    const auto                         srcPipelineStage    = GetBufferBarrierPipelineStage(srcStage);
    const auto                         dstPipelineStage    = GetBufferBarrierPipelineStage(dstStage);

    std::vector<VkBufferMemoryBarrier> bufferBarriers;
    for (const MBuffer* pBuffer: vBuffers)
    {
        VkBufferMemoryBarrier bufferBarrier;
        bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferBarrier.pNext               = nullptr;
        bufferBarrier.srcAccessMask       = srcAccessMask;
        bufferBarrier.dstAccessMask       = dstAccessMask;
        bufferBarrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        bufferBarrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        bufferBarrier.buffer              = pBuffer->m_vkBuffer;
        bufferBarrier.offset              = 0;
        bufferBarrier.size                = VK_WHOLE_SIZE;
        bufferBarriers.push_back(bufferBarrier);
    }

    vkCmdPipelineBarrier(
            m_vkCommandBuffer,
            srcPipelineStage,
            dstPipelineStage,
            0,
            0,
            nullptr,
            static_cast<uint32_t>(bufferBarriers.size()),
            bufferBarriers.data(),
            0,
            nullptr
    );


    return true;
}

VkPipelineStageFlags GetSrcPipelineStageFlags(VkImageLayout imageLayout)
{
    switch (imageLayout)
    {
        case VK_IMAGE_LAYOUT_GENERAL: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_PIPELINE_STAGE_TRANSFER_BIT;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        case VK_IMAGE_LAYOUT_UNDEFINED: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

        default: return VK_PIPELINE_STAGE_NONE_KHR;
    }

    return VK_PIPELINE_STAGE_NONE_KHR;
}

VkPipelineStageFlags GetDstPipelineStageFlags(VkImageLayout imageLayout)
{
    switch (imageLayout)
    {
        case VK_IMAGE_LAYOUT_GENERAL: return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_PIPELINE_STAGE_TRANSFER_BIT;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            return VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

        default: return VK_PIPELINE_STAGE_NONE_KHR;
    }

    return VK_PIPELINE_STAGE_NONE_KHR;
}

void MRenderCommandVulkan::SetTextureLayout(
        const std::vector<MTexture*>&     vTextures,
        const std::vector<VkImageLayout>& newLayouts
)
{
    std::vector<VkImageMemoryBarrier> vImageBarrier;

    VkPipelineStageFlags              srcPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;
    VkPipelineStageFlags              dstPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;

    for (size_t nTexIdx = 0; nTexIdx < vTextures.size(); ++nTexIdx)
    {
        MTexture* pTexture = vTextures[nTexIdx];
        if (pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderPresent) continue;

        auto textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();
        if (textureRHI->vkTextureImage == VK_NULL_HANDLE) { continue; }

        VkImageLayout oldLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        auto          findResult = m_textureLayout.find(pTexture);
        if (findResult != m_textureLayout.end()) oldLayout = findResult->second;

        if (oldLayout == newLayouts[nTexIdx]) continue;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask     = m_device->GetAspectFlags(textureRHI->vkTextureFormat);
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = pTexture->GetMipmapLevel();
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = pTexture->GetLayer();

        vImageBarrier.push_back(VkImageMemoryBarrier());
        VkImageMemoryBarrier& imageMemoryBarrier = vImageBarrier.back();

        m_device->TransitionLayoutBarrier(
                imageMemoryBarrier,
                textureRHI->vkTextureImage,
                oldLayout,
                newLayouts[nTexIdx],
                subresourceRange
        );
        textureRHI->vkImageLayout = newLayouts[nTexIdx];

        m_textureLayout[pTexture] = newLayouts[nTexIdx];

        srcPipelineStage |= GetSrcPipelineStageFlags(oldLayout);
        dstPipelineStage |= GetDstPipelineStageFlags(newLayouts[nTexIdx]);
    }

    if (vImageBarrier.empty()) return;

    vkCmdPipelineBarrier(
            m_vkCommandBuffer,
            srcPipelineStage,
            dstPipelineStage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            static_cast<uint32_t>(vImageBarrier.size()),
            vImageBarrier.data()
    );
}

bool MRenderCommandVulkan::DownloadTexture(
        MTexture*                                                         pTexture,
        const uint32_t&                                                   unMipIdx,
        const std::function<void(void* pImageData, const Vector2& size)>& callback
)
{
    if (!pTexture) { return false; }

    auto     textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();

    uint32_t unValidMipIdx = unMipIdx;
    if (unValidMipIdx >= pTexture->GetMipmapLevel())
    {
        MORTY_ASSERT(pTexture->GetMipmapLevel() > 0);
        unValidMipIdx = pTexture->GetMipmapLevel() - 1;
    }

    Vector3i size         = pTexture->GetSize();
    VkImage  textureImage = textureRHI->vkTextureImage;

    uint32_t unBufferWidth  = size.x;
    uint32_t unBufferHeight = size.y;
    uint32_t unBufferDepth  = size.z;

    for (uint32_t i = 0; i < unValidMipIdx; ++i)
    {
        if (unBufferWidth > 1) unBufferWidth /= 2;
        if (unBufferHeight > 1) unBufferHeight /= 2;
    }

    uint32_t unBufferSize =
            unBufferWidth * unBufferHeight * unBufferDepth * MTexture::GetImageMemorySize(pTexture->GetFormat());


    uint32_t   unMemoryID = MGlobal::M_INVALID_INDEX;
    MemoryInfo memoryInfo;
    VkBuffer   readBackBuffer = m_device->m_BufferPool.GetReadBackBuffer();
    if (!m_device->m_BufferPool.AllowReadBackBuffer(unBufferSize, unMemoryID, memoryInfo)) { return false; }

    VkBufferImageCopy region               = {};
    region.bufferOffset                    = memoryInfo.begin;
    region.bufferRowLength                 = unBufferWidth;
    region.bufferImageHeight               = unBufferHeight;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = unValidMipIdx;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = pTexture->GetLayer();
    region.imageOffset.x                   = 0;
    region.imageOffset.y                   = 0;
    region.imageOffset.z                   = 0;
    region.imageExtent.width               = unBufferWidth;
    region.imageExtent.height              = unBufferHeight;// copy to size
    region.imageExtent.depth               = unBufferDepth;


    SetTextureLayout({pTexture}, {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL});

    vkCmdCopyImageToBuffer(
            m_vkCommandBuffer,
            textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            readBackBuffer,
            1,
            &region
    );

    m_renderFinishedCallback.push_back([=]() {
        MByte* data = m_device->m_BufferPool.GetReadBackMemory();
        callback(data + memoryInfo.begin, Vector2(unBufferWidth, unBufferHeight));

        m_device->m_BufferPool.FreeReadBackBuffer(unMemoryID);
    });

    return true;
}

bool MRenderCommandVulkan::CopyImageBuffer(MTexture* pSource, MTexture* pTarget)
{
    if (!pSource || !pTarget) return false;

    SetTextureLayout({pSource}, {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL});
    SetTextureLayout({pTarget}, {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL});

    auto        sourceRHI = pSource->GetTextureRHI<MTextureRHIVulkan>();
    auto        targetRHI = pTarget->GetTextureRHI<MTextureRHIVulkan>();

    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {static_cast<int32_t>(pSource->GetSize().x), static_cast<int32_t>(pSource->GetSize().y), 1};
    blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel       = 0;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount     = pSource->GetLayer();
    blit.dstOffsets[0]                 = {0, 0, 0};
    blit.dstOffsets[1] = {static_cast<int32_t>(pTarget->GetSize().x), static_cast<int32_t>(pTarget->GetSize().y), 1};
    blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel       = 0;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount     = pTarget->GetLayer();

    vkCmdBlitImage(
            m_vkCommandBuffer,
            sourceRHI->vkTextureImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            targetRHI->vkTextureImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR
    );


    return true;
}

void MRenderCommandVulkan::ResetBuffer(const MBuffer* pBuffer)
{
    vkCmdFillBuffer(m_vkCommandBuffer, pBuffer->m_vkBuffer, 0, pBuffer->GetSize(), 0);
}

void MRenderCommandVulkan::UploadBuffer(MBuffer* pBuffer, const MByte* pData, const size_t nSize)
{
    MORTY_ASSERT(pBuffer);

    if (pBuffer->GetSize() < nSize)
    {
        pBuffer->ReallocMemory(nSize);
        m_device->DestroyBuffer(pBuffer);
        m_device->GenerateBuffer(pBuffer, pData, nSize);
    }
    else if (nSize > 0) { m_device->UploadBuffer(pBuffer, 0, pData, nSize); }
}

void MRenderCommandVulkan::addFinishedCallback(std::function<void()> func) { m_renderFinishedCallback.push_back(func); }

void MRenderCommandVulkan::UpdateShaderParam(std::shared_ptr<MShaderConstantParam> param)
{
    if (VK_NULL_HANDLE == param->m_vkBuffer)
    {
        //m_device->DestroyShaderParamBuffer(param);
        m_device->GenerateShaderParamBuffer(param);
    }

    //m_device->DestroyShaderParamBuffer(param);
    //m_device->GenerateShaderParamBuffer(param);

    MORTY_ASSERT(param->m_memoryMapping);

    if (param->m_memoryMapping)
    {
        memcpy(param->m_memoryMapping + param->m_unMemoryOffset, param->var.GetData(), param->var.GetSize());

#ifndef MORTY_WIN
        size_t nFlushMinSize = m_device->GetPhysicalDeviceProperties().limits.nonCoherentAtomSize;
        size_t nOffset       = (param->m_unMemoryOffset / nFlushMinSize) * nFlushMinSize;
        size_t nSize         = ((param->m_unMemoryOffset + param->m_unVkMemorySize) - nOffset);
        nSize                = nSize % nFlushMinSize == 0 ? nSize : (nSize / nFlushMinSize + 1) * nFlushMinSize;

        VkMappedMemoryRange memoryRange = {};
        memoryRange.sType               = VkStructureType::VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory              = param->m_vkBufferMemory;
        memoryRange.offset              = nOffset;
        memoryRange.size                = nSize;
        vkFlushMappedMemoryRanges(m_device->m_vkDevice, 1, &memoryRange);
#endif
    }
}

MVulkanPrimaryRenderCommand::MVulkanPrimaryRenderCommand()
    : MRenderCommandVulkan()
{
    m_vkRenderFinishedFence     = VK_NULL_HANDLE;
    m_vkRenderFinishedSemaphore = VK_NULL_HANDLE;

    m_renderWaitSemaphore = {};

    m_finished = false;
}

void MVulkanPrimaryRenderCommand::MarkFinished() { m_finished = true; }

void MVulkanPrimaryRenderCommand::OnCommandFinished()
{
    for (auto& callback: m_renderFinishedCallback) { callback(); }

    m_renderFinishedCallback.clear();
}

IRenderCommand* MVulkanPrimaryRenderCommand::CreateChildCommand()
{
    MVulkanSecondaryRenderCommand* pChildCommand = m_device->CreateChildCommand(this);
    m_secondaryCommand.push_back(pChildCommand);

    return pChildCommand;
}

IRenderCommand* MVulkanPrimaryRenderCommand::GetChildCommand(const size_t& nIndex)
{
    if (nIndex < m_secondaryCommand.size()) return m_secondaryCommand[nIndex];
    return nullptr;
}

void MVulkanPrimaryRenderCommand::ExecuteChildCommand()
{
    std::vector<VkCommandBuffer> buffers;
    for (MVulkanSecondaryRenderCommand* pChildCommand: m_secondaryCommand)
        buffers.push_back(pChildCommand->m_vkCommandBuffer);

    vkCmdExecuteCommands(m_vkCommandBuffer, static_cast<uint32_t>(buffers.size()), buffers.data());
}

VkImageLayout MRenderCommandVulkan::GetTextureBarrierLayout(MTexture* pTexture, METextureBarrierStage stage) const
{
    static const std::unordered_map<METextureBarrierStage, VkImageLayout> ImageLayoutTable = {
            {METextureBarrierStage::EPixelShaderSample, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
            {METextureBarrierStage::EComputeShaderWrite, VK_IMAGE_LAYOUT_GENERAL},
            {METextureBarrierStage::EShadingRateMask, VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR},
            {METextureBarrierStage::EComputeShaderRead, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    };

    if (stage == METextureBarrierStage::EPixelShaderWrite) { return m_device->GetImageLayout(pTexture); }

    const auto layout = ImageLayoutTable.find(stage);
    MORTY_ASSERT(layout != ImageLayoutTable.end());

    return layout->second;
}

VkAccessFlags MRenderCommandVulkan::GetBufferBarrierAccessFlag(MEBufferBarrierStage stage) const
{
    static const std::unordered_map<MEBufferBarrierStage, VkAccessFlags> AccessFlagTable = {
            {MEBufferBarrierStage::EComputeShaderWrite, VK_ACCESS_SHADER_WRITE_BIT},
            {MEBufferBarrierStage::EComputeShaderRead, VK_ACCESS_SHADER_READ_BIT},
            {MEBufferBarrierStage::EPixelShaderWrite, VK_ACCESS_SHADER_WRITE_BIT},
            {MEBufferBarrierStage::EPixelShaderRead, VK_ACCESS_SHADER_READ_BIT},
            {MEBufferBarrierStage::EDrawIndirectRead, VK_ACCESS_INDIRECT_COMMAND_READ_BIT},
    };

    const auto accessMask = AccessFlagTable.find(stage);
    MORTY_ASSERT(accessMask != AccessFlagTable.end());

    return accessMask->second;
}

VkPipelineStageFlags MRenderCommandVulkan::GetBufferBarrierPipelineStage(MEBufferBarrierStage stage) const
{
    static const std::unordered_map<MEBufferBarrierStage, VkPipelineStageFlags> PipelineStageTable = {
            {MEBufferBarrierStage::EComputeShaderWrite, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
            {MEBufferBarrierStage::EComputeShaderRead, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
            {MEBufferBarrierStage::EPixelShaderWrite, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {MEBufferBarrierStage::EPixelShaderRead, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {MEBufferBarrierStage::EDrawIndirectRead, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT},
            {MEBufferBarrierStage::EShadingRateRead, VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR},
    };

    const auto pipelineStage = PipelineStageTable.find(stage);
    MORTY_ASSERT(pipelineStage != PipelineStageTable.end());

    return pipelineStage->second;
}

VkPipelineStageFlags MRenderCommandVulkan::GetTextureBarrierPipelineStage(METextureBarrierStage stage) const
{
    static const std::unordered_map<METextureBarrierStage, VkPipelineStageFlags> PipelineStageTable = {
            {METextureBarrierStage::EPixelShaderSample, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {METextureBarrierStage::EPixelShaderWrite, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
            {METextureBarrierStage::EComputeShaderWrite, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
            {METextureBarrierStage::EComputeShaderRead, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT},
    };

    const auto pipelineStage = PipelineStageTable.find(stage);
    MORTY_ASSERT(pipelineStage != PipelineStageTable.end());

    return pipelineStage->second;
}


MRenderPassCmd MRenderCommandVulkan::BeginRenderPass(MRenderPass* renderPass)
{
    return MRenderPassCmd(m_device, renderPass);
}

void MRenderCommandVulkan::EndRenderPass(const MRenderPassCmd& commands)
{
    for (const auto command: commands.GetCommand()) { m_executeTable.PerProcess(this, command); }

    InternalBeginRenderPass(commands.GetRenderPass());

    for (const auto command: commands.GetCommand()) { m_executeTable.RunCommand(this, command); }

    InternalEndRenderPass();
}
