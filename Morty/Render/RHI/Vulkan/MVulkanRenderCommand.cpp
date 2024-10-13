#include "MVulkanRenderCommand.h"

#include "MVulkanPhysicalDevice.h"
#include "Material/MComputeDispatcher.h"
#include "Material/MMaterial.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertexBuffer.h"
#include "RHI/Vulkan/MTextureRHIVulkan.h"

using namespace morty;

void MVulkanRenderCommand::SetViewport(const MViewportInfo& viewport)
{
    VkViewport vkViewport = {};
    vkViewport.x          = viewport.x;
    vkViewport.y          = viewport.y + viewport.height;
    vkViewport.width      = std::max(viewport.width, 1.0f);
    vkViewport.height     = -viewport.height;
    vkViewport.minDepth   = viewport.minz;
    vkViewport.maxDepth   = viewport.maxz;

    vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &vkViewport);
}

void MVulkanRenderCommand::SetScissor(const MScissorInfo& scissor)
{
    float    width  = std::max(scissor.width, 1.0f);
    float    height = std::max(scissor.height, 1.0f);

    VkRect2D scissorRect = {
            VkOffset2D{int32_t(scissor.x), int32_t(scissor.y)},
            VkExtent2D{uint32_t(width), uint32_t(height)}
    };

    vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &scissorRect);
}

void MVulkanRenderCommand::RenderCommandBegin()
{
    vkResetCommandBuffer(m_vkCommandBuffer, 0);

    //CommandBuffer Begin Info
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    //Begin Command Buffer
    vkBeginCommandBuffer(m_vkCommandBuffer, &beginInfo);

    m_drawCallCount = 0;

    pUsingPipeline = nullptr;
}

void MVulkanRenderCommand::RenderCommandEnd()
{
    //End Command Buffer
    vkEndCommandBuffer(m_vkCommandBuffer);
}

void MVulkanRenderCommand::BeginRenderPass(MRenderPass* pRenderPass)
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
    for (uint32_t i = 0; i < unBackNum; ++i)
    {
        const MColor color = pRenderPass->m_renderTarget.backTargets[i].desc.cClearColor;
        //-Wmissing-braces
        vClearValues[i].color = {{color.r, color.g, color.b, color.a}};
    }

    if (MTexturePtr pTexture = pRenderPass->GetDepthTexture())
    {
        vClearValues.push_back({});
        vClearValues.back().depthStencil = {1.0f, 0};
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

    m_renderPassStages.push(MRenderPassStage(pRenderPass, 0));
}

void MVulkanRenderCommand::NextSubPass()
{
    if (m_renderPassStages.empty()) return;

    vkCmdNextSubpass(m_vkCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    m_renderPassStages.top().nSubpassIdx++;
}

void MVulkanRenderCommand::EndRenderPass()
{
    if (m_renderPassStages.empty()) return;

    m_renderPassStages.pop();

    //End Render Pass
    vkCmdEndRenderPass(m_vkCommandBuffer);
}

void MVulkanRenderCommand::DrawMesh(MIMesh* pMesh)
{
    if (!pMesh) return;

    DrawMesh(pMesh, 0, static_cast<uint32_t>(pMesh->GetIndicesNum()), 0);
}

void MVulkanRenderCommand::DrawMesh(
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

void MVulkanRenderCommand::DrawMesh(
        const MBuffer* pVertexBuffer,
        const MBuffer* pIndexBuffer,
        const size_t   nVertexOffset,
        const size_t   nIndexOffset,
        const size_t   nIndexCount
)
{
    if (!pVertexBuffer || !pIndexBuffer)
    {
        MORTY_ASSERT(pVertexBuffer && pIndexBuffer);
        return;
    }

    if (pVertexBuffer->m_vkBuffer == VK_NULL_HANDLE || pIndexBuffer->m_vkBuffer == VK_NULL_HANDLE)
    {
        MORTY_ASSERT(pVertexBuffer->m_vkBuffer != VK_NULL_HANDLE && pIndexBuffer->m_vkBuffer != VK_NULL_HANDLE);
        return;
    }

    if (pUsingVertex != pVertexBuffer)
    {
        const VkBuffer         vertexBuffers[] = {pVertexBuffer->m_vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = pVertexBuffer;
    }

    if (pUsingIndex != pIndexBuffer)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, pIndexBuffer->m_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = pIndexBuffer;
    }

    vkCmdDrawIndexed(
            m_vkCommandBuffer,
            static_cast<uint32_t>(nIndexCount),
            1,
            static_cast<uint32_t>(nIndexOffset),
            static_cast<uint32_t>(nVertexOffset),
            0
    );

    ++m_drawCallCount;
}

void MVulkanRenderCommand::DrawIndexedIndirect(
        const MBuffer* pVertexBuffer,
        const MBuffer* pIndexBuffer,
        const MBuffer* pCommandsBuffer,
        const size_t&  offset,
        const size_t&  count
)
{
    if (pUsingVertex != pVertexBuffer)
    {
        const VkBuffer         vertexBuffers[] = {pVertexBuffer->m_vkBuffer};
        constexpr VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        pUsingVertex = pVertexBuffer;
    }

    if (pUsingIndex != pIndexBuffer)
    {
        vkCmdBindIndexBuffer(m_vkCommandBuffer, pIndexBuffer->m_vkBuffer, 0, VK_INDEX_TYPE_UINT32);
        pUsingIndex = pIndexBuffer;
    }

    if (m_device->MultiDrawIndirectSupport())
    {
        vkCmdDrawIndexedIndirect(
                m_vkCommandBuffer,
                pCommandsBuffer->m_vkBuffer,
                offset,
                static_cast<uint32_t>(count),
                sizeof(VkDrawIndexedIndirectCommand)
        );
    }
    else
    {
        for (size_t nDrawIdx = 0; nDrawIdx < count; ++nDrawIdx)
        {
            vkCmdDrawIndexedIndirect(
                    m_vkCommandBuffer,
                    pCommandsBuffer->m_vkBuffer,
                    offset + sizeof(VkDrawIndexedIndirectCommand) * nDrawIdx,
                    1,
                    sizeof(VkDrawIndexedIndirectCommand)
            );
        }
    }
    ++m_drawCallCount;
}

bool MVulkanRenderCommand::SetUseMaterial(std::shared_ptr<MMaterial> pMaterial)
{
    if (!SetGraphPipeline(pMaterial)) { return false; }

    if (!pMaterial) { return false; }

    std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = pMaterial->GetMaterialPropertyBlock();
    SetShaderPropertyBlock(pPropertyBlock);

    for (const auto& pPushedProperty: m_propertyBlockStack) { SetShaderPropertyBlock(pPushedProperty); }

    return true;
}

bool MVulkanRenderCommand::SetGraphPipeline(std::shared_ptr<MMaterial> pMaterial)
{
    if (m_renderPassStages.empty())
    {
        MORTY_ASSERT(!m_renderPassStages.empty());
        return false;
    }

    if (nullptr == pMaterial)
    {
        MORTY_ASSERT(nullptr != pMaterial);
        pUsingPipeline = nullptr;
        return false;
    }

    const MRenderPassStage stage     = m_renderPassStages.top();
    const auto             pPipeline = m_device->m_PipelineManager.FindOrCreateGraphicsPipeline(
            pMaterial->GetMaterialTemplate().get(),
            stage.pRenderPass
    );

    MORTY_ASSERT(nullptr != pPipeline);

    if (pUsingPipeline == pPipeline) { return true; }

    pUsingPipeline = pPipeline;
    pUsingVertex   = nullptr;
    pUsingIndex    = nullptr;

    const auto pGraphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pPipeline);

    VkPipeline vkPipeline = pGraphicsPipeline->GetSubpassPipeline(stage.nSubpassIdx);
    MORTY_ASSERT(vkPipeline);

    vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

    SetShadingRate(pMaterial->GetShadingRate(), {MEShadingRateCombinerOp::Max, MEShadingRateCombinerOp::Max});

    return true;
}

bool MVulkanRenderCommand::DispatchComputeJob(
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

    if (pUsingPipeline == pPipeline) { return true; }

    pUsingPipeline = pPipeline;

    if (std::shared_ptr<MComputePipeline> pComputePipeline = std::dynamic_pointer_cast<MComputePipeline>(pPipeline))
    {
        VkPipeline vkPipeline = pComputePipeline->m_vkPipeline;
        MORTY_ASSERT(VK_NULL_HANDLE != vkPipeline);

        vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline);

        for (const std::shared_ptr<MShaderPropertyBlock>& params: pComputeDispatcher->GetShaderPropertyBlocks())
        {
            SetShaderPropertyBlock(params);
        }

        vkCmdDispatch(m_vkCommandBuffer, nGroupX, nGroupY, nGroupZ);

        return true;
    }

    return false;
}

void MVulkanRenderCommand::SetShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
    bool bNeedAllocDescriptorSet = false;
    for (const auto& pParam: pPropertyBlock->m_params)
    {
        if (pParam->bDirty)
        {
            //	bNeedAllocDescriptorSet = true;

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
        m_device->m_PipelineManager.AllocateShaderPropertyBlock(pPropertyBlock, pUsingPipeline);

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

    MORTY_ASSERT(VK_NULL_HANDLE != pUsingPipeline->m_pipelineLayout.vkPipelineLayout);
    MORTY_ASSERT(VK_NULL_HANDLE != pPropertyBlock->m_vkDescriptorSet);

    std::vector<uint32_t> vDynamicOffsets;
    for (const auto& pParam: pPropertyBlock->m_params)
    {
        if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        {
            vDynamicOffsets.push_back(pParam->m_unMemoryOffset);
        }
    }

    VkPipelineBindPoint vkPipelineBindPoint = pUsingPipeline->m_vkPipelineBindPoint;
    vkCmdBindDescriptorSets(
            m_vkCommandBuffer,
            vkPipelineBindPoint,
            pUsingPipeline->m_pipelineLayout.vkPipelineLayout,
            pPropertyBlock->m_unKey,
            1,
            &pPropertyBlock->m_vkDescriptorSet,
            static_cast<uint32_t>(vDynamicOffsets.size()),
            vDynamicOffsets.data()
    );
}

void MVulkanRenderCommand::PushShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
    m_propertyBlockStack.push_back(pPropertyBlock);
}

void MVulkanRenderCommand::PopShaderPropertyBlock() { m_propertyBlockStack.pop_back(); }

bool MVulkanRenderCommand::AddRenderToTextureBarrier(
        const std::vector<MTexture*> vTextures,
        METextureBarrierStage        dstStage
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

bool MVulkanRenderCommand::AddBufferMemoryBarrier(
        const std::vector<const MBuffer*> vBuffers,
        MEBufferBarrierStage              srcStage,
        MEBufferBarrierStage              dstStage
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

void MVulkanRenderCommand::SetTextureLayout(
        const std::vector<MTexture*>&     vTextures,
        const std::vector<VkImageLayout>& newLayouts
)
{
    std::vector<VkImageMemoryBarrier> vImageBarrier;

    VkPipelineStageFlags              srcPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;
    VkPipelineStageFlags              dstPipelineStage = VK_PIPELINE_STAGE_NONE_KHR;

    for (size_t nTexIdx = 0; nTexIdx < vTextures.size(); ++nTexIdx)
    {
        MTexture*     pTexture   = vTextures[nTexIdx];
        auto          textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();

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

bool MVulkanRenderCommand::DownloadTexture(
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

bool MVulkanRenderCommand::CopyImageBuffer(MTexture* pSource, MTexture* pTarget)
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

void MVulkanRenderCommand::UpdateMipmaps(MTexture* pTexture)
{
    if (!pTexture) return;

    m_device->GenerateMipmaps(pTexture, pTexture->GetMipmapLevel(), m_vkCommandBuffer);
}

void MVulkanRenderCommand::ResetBuffer(const MBuffer* pBuffer)
{
    vkCmdFillBuffer(m_vkCommandBuffer, pBuffer->m_vkBuffer, 0, pBuffer->GetSize(), 0);
}

void MVulkanRenderCommand::UploadBuffer(MBuffer* pBuffer, const MByte* pData, const size_t nSize)
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

void MVulkanRenderCommand::FillTexture(MTexture* pTexture, MColor color)
{
    MORTY_UNUSED(color);

    auto                textureRHI = pTexture->GetTextureRHI<MTextureRHIVulkan>();

    const VkImageLayout vkClearLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    SetTextureLayout({pTexture}, {vkClearLayout});

    VkClearColorValue vkColor;
    memset(&vkColor, 0, sizeof(vkColor));

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask     = m_device->GetAspectFlags(textureRHI->vkTextureFormat);
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.levelCount     = pTexture->GetMipmapLevel();
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount     = pTexture->GetLayer();
    vkCmdClearColorImage(m_vkCommandBuffer, textureRHI->vkTextureImage, vkClearLayout, &vkColor, 1, &subresourceRange);
}

void MVulkanRenderCommand::addFinishedCallback(std::function<void()> func) { m_renderFinishedCallback.push_back(func); }

void MVulkanRenderCommand::SetShadingRate(Vector2i i2ShadingSize, std::array<MEShadingRateCombinerOp, 2> combineOp)
{
    const VkExtent2D vkShadingSize = {static_cast<uint32_t>(i2ShadingSize.x), static_cast<uint32_t>(i2ShadingSize.y)};
    const VkFragmentShadingRateCombinerOpKHR vkCombinerOp[2] = {
            m_device->GetShadingRateCombinerOp(combineOp[0]),
            m_device->GetShadingRateCombinerOp(combineOp[1])
    };

    m_device->GetPhysicalDevice()->vkCmdSetFragmentShadingRateKHR(m_vkCommandBuffer, &vkShadingSize, vkCombinerOp);
}

void MVulkanRenderCommand::UpdateBuffer(MBuffer* pBuffer, const MByte* data, const size_t& size)
{
    if (!pBuffer) { return; }

    if (pBuffer->m_stageType == MBuffer::MStageType::EWaitAllow)
    {
        pBuffer->DestroyBuffer(m_device);
        pBuffer->GenerateBuffer(m_device, data, size);
    }
    else if (pBuffer->m_stageType == MBuffer::MStageType::EWaitSync) { pBuffer->UploadBuffer(m_device, data, size); }
}

void MVulkanRenderCommand::UpdateShaderParam(std::shared_ptr<MShaderConstantParam> param)
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
    : MVulkanRenderCommand()
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

MIRenderCommand* MVulkanPrimaryRenderCommand::CreateChildCommand()
{
    MVulkanSecondaryRenderCommand* pChildCommand = m_device->CreateChildCommand(this);
    m_secondaryCommand.push_back(pChildCommand);

    return pChildCommand;
}

MIRenderCommand* MVulkanPrimaryRenderCommand::GetChildCommand(const size_t& nIndex)
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

VkImageLayout MVulkanRenderCommand::GetTextureBarrierLayout(MTexture* pTexture, METextureBarrierStage stage) const
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

VkAccessFlags MVulkanRenderCommand::GetBufferBarrierAccessFlag(MEBufferBarrierStage stage) const
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

VkPipelineStageFlags MVulkanRenderCommand::GetBufferBarrierPipelineStage(MEBufferBarrierStage stage) const
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

VkPipelineStageFlags MVulkanRenderCommand::GetTextureBarrierPipelineStage(METextureBarrierStage stage) const
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
