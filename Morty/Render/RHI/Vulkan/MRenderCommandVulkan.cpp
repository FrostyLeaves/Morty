#include "MRenderCommandVulkan.h"

#include "MBufferRHIVulkan.h"
#include "MVulkanCommandExecuteTable.h"
#include "MVulkanPhysicalDevice.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Mesh/MMesh.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/Vulkan/MTextureRHIVulkan.h"

using namespace morty;

MVulkanCommandExecuteTable MRenderCommandVulkan::m_executeTable = MVulkanCommandExecuteTable();


void                       MRenderCommandVulkan::SetViewport(const MSetViewportCmd* viewport) const
{
    VkViewport vkViewport = {};
    vkViewport.x          = viewport->rect.x;
    vkViewport.y          = viewport->rect.y + viewport->rect.height;
    vkViewport.width      = std::max(viewport->rect.width, 1);
    vkViewport.height     = -viewport->rect.height;
    vkViewport.minDepth   = viewport->minDepth;
    vkViewport.maxDepth   = viewport->maxDepth;

    vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &vkViewport);
}

void MRenderCommandVulkan::SetScissor(const MSetScissorCmd* scissor) const
{
    VkRect2D scissorRect = {
            VkOffset2D{int32_t(scissor->rect.x), int32_t(scissor->rect.y)},
            VkExtent2D{uint32_t(std::max(scissor->rect.width, 1)), uint32_t(std::max(scissor->rect.height, 1))}
    };

    vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &scissorRect);
}

void MRenderCommandVulkan::DrawMesh(const MDrawMeshCmd* cmd)
{
    auto vertex  = static_cast<const MBufferRHIVulkan*>(cmd->vertexBuffer);
    auto indices = static_cast<const MBufferRHIVulkan*>(cmd->indexBuffer);

    MORTY_ASSERT(vertex != VK_NULL_HANDLE && indices != VK_NULL_HANDLE);

    if (pUsingVertex != vertex)
    {
        const VkBuffer         vertexBuffers[] = {vertex->vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = vertex;
    }

    if (pUsingIndex != indices)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, indices->vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = indices;
    }

    vkCmdDrawIndexed(m_vkCommandBuffer, cmd->indexCount, 1, cmd->indexOffset, cmd->vertexOffset, 0);

    ++m_drawCallCount;
}

void MRenderCommandVulkan::DrawIndexedIndirect(const MDrawIndexedIndirectCmd* cmd)
{
    auto vertex   = static_cast<const MBufferRHIVulkan*>(cmd->vertexBuffer);
    auto indices  = static_cast<const MBufferRHIVulkan*>(cmd->indexBuffer);
    auto commands = static_cast<const MBufferRHIVulkan*>(cmd->commandsBuffer);

    if (pUsingVertex != vertex)
    {
        const VkBuffer         vertexBuffers[] = {vertex->vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = vertex;
    }

    if (pUsingIndex != indices)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, indices->vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = indices;
    }

    if (m_device->MultiDrawIndirectSupport())
    {
        vkCmdDrawIndexedIndirect(
                m_vkCommandBuffer,
                commands->vkBuffer,
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
                    commands->vkBuffer,
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

void MRenderCommandVulkan::SetShaderParameterSet(const MSetShaderParameterSetCmd* cmd)
{
    auto pParameterSet      = cmd->property;
    auto pPipeline          = cmd->pipeline;
    auto allocDescriptorSet = cmd->allocDescriptorSet;

    if (VK_NULL_HANDLE == pParameterSet->m_vkDescriptorSet) { allocDescriptorSet = true; }

    if (allocDescriptorSet)
    {
        //alloc a new descriptor set.
        m_device->m_PipelineManager.AllocateShaderParameterSet(pParameterSet, pPipeline);

        std::vector<VkWriteDescriptorSet> vWriteDescriptorSet;

        for (const auto& param: pParameterSet->GetConstantParams())
        {
            // bind buffer to descriptor set.
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindConstantParam(param.get(), writeDescriptorSet);
            writeDescriptorSet.dstSet = pParameterSet->m_vkDescriptorSet;
        }

        for (const auto& param: pParameterSet->GetTextureParams())
        {
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindTextureParam(param.get(), writeDescriptorSet);
            writeDescriptorSet.dstSet = pParameterSet->m_vkDescriptorSet;
        }

        for (const auto& param: pParameterSet->GetStorageParams())
        {
            vWriteDescriptorSet.push_back({});
            VkWriteDescriptorSet& writeDescriptorSet = vWriteDescriptorSet.back();
            m_device->m_PipelineManager.BindStorageParam(param.get(), writeDescriptorSet);
            writeDescriptorSet.dstSet = pParameterSet->m_vkDescriptorSet;
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
    MORTY_ASSERT(VK_NULL_HANDLE != pParameterSet->m_vkDescriptorSet);

    std::vector<uint32_t> vDynamicOffsets;
    for (const auto& pParam: pParameterSet->GetConstantParams())
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
            pParameterSet->m_unKey,
            1,
            &pParameterSet->m_vkDescriptorSet,
            static_cast<uint32_t>(vDynamicOffsets.size()),
            vDynamicOffsets.data()
    );
}

void MRenderCommandVulkan::AddBarrierForPixelSample(const MSetShaderParameterSetCmd* cmd)
{
    std::vector<MTexture*> vTextures;
    for (const auto& pParam: cmd->property->GetTextureParams())
    {
        if (auto texture = pParam->GetTexture().get()) { vTextures.emplace_back(texture); }
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
        vBackTextures[i]      = pRenderPass->m_renderTarget.backTargets[i].texture.get();
    }
    AddRenderToTextureBarrier(vBackTextures, METextureBarrierStage::EPixelShaderWrite);


    if (MTexturePtr texture = pRenderPass->GetDepthTexture())
    {
        vClearValues.push_back({});
        vClearValues.back().depthStencil = {1.0f, 0};

        AddRenderToTextureBarrier({texture.get()}, METextureBarrierStage::EPixelShaderWrite);
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
        MIMesh*         mesh,
        const uint32_t& nIdxOffset,
        const uint32_t& nIdxCount,
        const uint32_t& nVrtOffset
)
{
    if (!mesh) return;

    if (0 == nIdxCount) return;

    MBuffer* pVertexBuffer = mesh->GetVertexBuffer();
    MBuffer* pIndexBuffer  = mesh->GetIndexBuffer();

    if (!pVertexBuffer || !pIndexBuffer) { return; }

    UpdateBuffer(pVertexBuffer, mesh->GetVerticesVector().data(), mesh->GetVerticesVector().size());
    UpdateBuffer(pIndexBuffer, mesh->GetIndicesVector().data(), mesh->GetIndicesVector().size());

    DrawMesh(pVertexBuffer, pIndexBuffer, nVrtOffset, nIdxOffset, nIdxCount);
}
 */

bool MRenderCommandVulkan::DispatchComputeJob(
        MComputeDispatcher* pComputeDispatcher,
        const MStringId&    entryName,
        const uint32_t&     nGroupX,
        const uint32_t&     nGroupY,
        const uint32_t&     nGroupZ
)
{
    MORTY_UNUSED(entryName);
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

        for (const std::shared_ptr<MShaderParameterSet>& params: pComputeDispatcher->GetShaderParameterSets())
        {
            MSetShaderParameterSetCmd cmd{
                    .pipeline           = pComputePipeline.get(),
                    .property           = params.get(),
                    .allocDescriptorSet = m_device->SyncParameterSet(params.get()),
            };

            SetShaderParameterSet(&cmd);
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
    std::transform(vTextures.begin(), vTextures.end(), layouts.begin(), [this, dstStage](auto texture) {
        return GetTextureBarrierLayout(texture, dstStage);
    });


    SetTextureLayout(vTextures, layouts);
    return true;
}

bool MRenderCommandVulkan::AddBufferMemoryBarrier(
        const std::vector<const MBufferRHI*>& vBuffers,
        MEBufferBarrierStage                  srcStage,
        MEBufferBarrierStage                  dstStage
)
{
    const auto                         srcAccessMask       = GetBufferBarrierAccessFlag(srcStage);
    const auto                         dstAccessMask       = GetBufferBarrierAccessFlag(dstStage);
    const uint32_t                     srcQueueFamilyIndex = m_device->GetBufferBarrierQueueFamily(srcStage);
    const uint32_t                     dstQueueFamilyIndex = m_device->GetBufferBarrierQueueFamily(dstStage);
    const auto                         srcPipelineStage    = GetBufferBarrierPipelineStage(srcStage);
    const auto                         dstPipelineStage    = GetBufferBarrierPipelineStage(dstStage);

    std::vector<VkBufferMemoryBarrier> bufferBarriers;
    for (const auto* buffer: vBuffers)
    {
        const auto*           bufferRHI = static_cast<const MBufferRHIVulkan*>(buffer);

        VkBufferMemoryBarrier bufferBarrier;
        bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufferBarrier.pNext               = nullptr;
        bufferBarrier.srcAccessMask       = srcAccessMask;
        bufferBarrier.dstAccessMask       = dstAccessMask;
        bufferBarrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        bufferBarrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        bufferBarrier.buffer              = bufferRHI->vkBuffer;
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
        MTexture* texture = vTextures[nTexIdx];
        if (texture->GetWriteUsage() & METextureWriteUsageBit::ERenderPresent) continue;

        auto textureRHI = texture->GetTextureRHI<MTextureRHIVulkan>();
        if (textureRHI->vkTextureImage == VK_NULL_HANDLE) { continue; }

        VkImageLayout oldLayout  = textureRHI->vkImageLayout;
        auto          findResult = m_textureLayout.find(texture);
        if (findResult != m_textureLayout.end()) oldLayout = findResult->second;

        if (oldLayout == newLayouts[nTexIdx]) continue;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask     = morty::MVulkanDevice::GetAspectFlags(textureRHI->vkTextureFormat);
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = texture->GetMipmapLevel();
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = texture->GetLayer();

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

        m_textureLayout[texture] = newLayouts[nTexIdx];

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
        MTexture*                                                         texture,
        const uint32_t&                                                   unMipIdx,
        const std::function<void(void* pImageData, const Vector2& size)>& callback
)
{
    if (!texture) { return false; }

    auto     textureRHI = texture->GetTextureRHI<MTextureRHIVulkan>();

    uint32_t unValidMipIdx = unMipIdx;
    if (unValidMipIdx >= texture->GetMipmapLevel())
    {
        MORTY_ASSERT(texture->GetMipmapLevel() > 0);
        unValidMipIdx = texture->GetMipmapLevel() - 1;
    }

    Vector3i size         = texture->GetSize();
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
            unBufferWidth * unBufferHeight * unBufferDepth * MTexture::GetImageMemorySize(texture->GetFormat());


    uint32_t   unMemoryID = MGlobal::M_INVALID_UINDEX;
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
    region.imageSubresource.layerCount     = texture->GetLayer();
    region.imageOffset.x                   = 0;
    region.imageOffset.y                   = 0;
    region.imageOffset.z                   = 0;
    region.imageExtent.width               = unBufferWidth;
    region.imageExtent.height              = unBufferHeight;// copy to size
    region.imageExtent.depth               = unBufferDepth;


    SetTextureLayout({texture}, {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL});

    vkCmdCopyImageToBuffer(
            m_vkCommandBuffer,
            textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            readBackBuffer,
            1,
            &region
    );

    m_renderFinishedCallback.push_back([=, this]() {
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

void MRenderCommandVulkan::addFinishedCallback(std::function<void()> func) { m_renderFinishedCallback.push_back(func); }

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

VkImageLayout MRenderCommandVulkan::GetTextureBarrierLayout(MTexture* texture, METextureBarrierStage stage) const
{
    static const std::unordered_map<METextureBarrierStage, VkImageLayout> ImageLayoutTable = {
            {METextureBarrierStage::EPixelShaderSample, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
            {METextureBarrierStage::EComputeShaderWrite, VK_IMAGE_LAYOUT_GENERAL},
            {METextureBarrierStage::EShadingRateMask, VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR},
            {METextureBarrierStage::EComputeShaderRead, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    };

    if (stage == METextureBarrierStage::EPixelShaderWrite) { return m_device->GetImageLayout(texture); }

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
