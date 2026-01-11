#include "MRenderPassCmd.h"
#include "Material/MMaterial.h"
#include "Material/MMaterialPass.h"
#include "Mesh/MMesh.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MPipeline.h"

using namespace morty;

MRenderPassCmd::MRenderPassCmd(MIDevice* device, MRenderPass* renderPass)
    : m_device(device)
    , m_renderPass(renderPass)
{}

MRenderPassCmd::~MRenderPassCmd()
{
    for (auto cmd: m_commandQueue) { delete cmd; }
}

void MRenderPassCmd::DrawMesh(
        const MBuffer* pVertexBuffer,
        const MBuffer* pIndexBuffer,
        size_t         nVertexOffset,
        size_t         nIndexOffset,
        size_t         nIndexCount
)
{
    m_commandQueue.emplace_back(new MDrawMeshCmd(
            {.vertexBuffer = pVertexBuffer->m_bufferRHI.get(),
             .indexBuffer  = pIndexBuffer->m_bufferRHI.get(),
             .vertexOffset = static_cast<uint32_t>(nVertexOffset),
             .indexOffset  = static_cast<uint32_t>(nIndexOffset),
             .indexCount   = static_cast<uint32_t>(nIndexCount)}
    ));
}

void MRenderPassCmd::DrawIndexedIndirect(
        const MBuffer* pVertexBuffer,
        const MBuffer* pIndexBuffer,
        const MBuffer* pCommandsBuffer,
        size_t         offset,
        size_t         count
)
{
    m_commandQueue.emplace_back(new MDrawIndexedIndirectCmd{
            .vertexBuffer   = pVertexBuffer->m_bufferRHI.get(),
            .indexBuffer    = pIndexBuffer->m_bufferRHI.get(),
            .commandsBuffer = pCommandsBuffer->m_bufferRHI.get(),
            .offset         = offset,
            .count          = count,
    });
}

void MRenderPassCmd::DrawIndexedIndirectCount(
        const MBuffer* vertexBuffer,
        const MBuffer* indexBuffer,
        const MBuffer* commandsBuffer,
        const MBuffer* countBuffer,
        size_t         commandOffset,
        size_t         countOffset,
        size_t         maxCount
)
{
    m_commandQueue.emplace_back(new MDrawIndexedIndirectCountCmd{
            .vertexBuffer   = vertexBuffer->m_bufferRHI.get(),
            .indexBuffer    = indexBuffer->m_bufferRHI.get(),
            .commandsBuffer = commandsBuffer->m_bufferRHI.get(),
            .countBuffer    = countBuffer->m_bufferRHI.get(),
            .commandOffset  = commandOffset,
            .countOffset    = countOffset,
            .maxCount       = maxCount
    });
}

void MRenderPassCmd::SetGraphPipeline(const MGraphicsPipeline* pipeline, size_t subPassIdx)
{
    m_commandQueue.emplace_back(new MSetGraphPipelineCmd{
            .pipeline   = pipeline,
            .subPassIdx = subPassIdx,
    });
}

void MRenderPassCmd::SetShaderParameterSet(const std::shared_ptr<MShaderParameterSet>& block)
{
    SetShaderParameterSet(block.get());
}

void MRenderPassCmd::SetShaderParameterSet(MShaderParameterSet* propertyBlock)
{
    m_commandQueue.emplace_back(new MSetShaderParameterSetCmd{
            .pipeline           = m_usingPipeline,
            .property           = propertyBlock,
            .allocDescriptorSet = m_device->SyncParameterSet(propertyBlock),
    });
}

void MRenderPassCmd::PushShaderParameterSet(MShaderParameterSet* pParameterSet)
{
    m_propertyBlockStack.emplace_back(pParameterSet);
}

void MRenderPassCmd::PopShaderParameterSet() { m_propertyBlockStack.pop_back(); }

void MRenderPassCmd::ApplyPushedShaderParameterSets()
{
    for (const auto& pPushedProperty: m_propertyBlockStack) { SetShaderParameterSet(pPushedProperty); }
}

void MRenderPassCmd::AddTextureBarrier(const std::vector<MTexture*>& vTextures, METextureBarrierStage dstStage)
{
    m_commandQueue.emplace_back(new MAddTextureBarrierCmd{
            .textures = vTextures,
            .dstStage = dstStage,
    });
}

void MRenderPassCmd::SetViewport(const MSetViewportCmd& viewport)
{
    m_commandQueue.emplace_back(new MSetViewportCmd(viewport));
}

void MRenderPassCmd::SetScissor(const MSetScissorCmd& scissor)
{
    m_commandQueue.emplace_back(new MSetScissorCmd(scissor));
}

void MRenderPassCmd::SetViewportAndScissor(const MSetViewportCmd& viewport)
{
    m_commandQueue.emplace_back(new MSetViewportCmd(viewport));
    m_commandQueue.emplace_back(new MSetScissorCmd{.rect = viewport.rect});
}

void MRenderPassCmd::NextSubPass()
{
    m_commandQueue.emplace_back(new MNextSubPassCmd{});

    ++m_subPassIdx;
}

void MRenderPassCmd::SetShadingRate(Vector2i i2ShadingSize, const std::array<MEShadingRateCombinerOp, 2>& combineOp)
{
    m_commandQueue.emplace_back(new MSetShadingRateCmd{
            .shadingRate = i2ShadingSize,
            .combineOp   = combineOp,
    });
}

void MRenderPassCmd::DrawMesh(MIMesh* mesh, size_t nIndexOffset, size_t nIndexCount, size_t nVertexOffset)
{
    if (!m_usingPipeline) return;
    if (!mesh) return;

    MBuffer* pVertexBuffer = mesh->GetVertexBuffer();
    MBuffer* pIndexBuffer  = mesh->GetIndexBuffer();

    if (!pVertexBuffer || !pIndexBuffer) { return; }

    UpdateBuffer(pVertexBuffer, mesh->GetVerticesVector().data(), mesh->GetVerticesVector().size());
    UpdateBuffer(
            pIndexBuffer,
            reinterpret_cast<const MByte*>(mesh->GetIndicesVector().data()),
            mesh->GetIndicesVector().size() * mesh->GetIndexStructSize()
    );

    DrawMesh(pVertexBuffer, pIndexBuffer, nVertexOffset, nIndexOffset, nIndexCount);
}

void MRenderPassCmd::DrawMesh(MIMesh* mesh) { DrawMesh(mesh, 0, mesh->GetIndicesNum(), 0); }


void MRenderPassCmd::SetGraphPipeline(const MMaterialPass* pass)
{
    if (pass == nullptr)
    {
        m_usingPipeline = nullptr;
        return;
    }

    const auto pPipeline = m_device->FindOrCreateGraphicsPipeline(pass, m_renderPass);
    MORTY_ASSERT(nullptr != pPipeline);
    if (m_usingPipeline == pPipeline.get()) { return; }
    m_usingPipeline = pPipeline.get();

    const auto pGraphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pPipeline);

    SetGraphPipeline(pGraphicsPipeline.get(), m_subPassIdx);
    //SetShadingRate(material->GetShadingRate(), {MEShadingRateCombinerOp::Max, MEShadingRateCombinerOp::Max});
}

void MRenderPassCmd::SetMaterial(const MMaterial* material, const MMaterialPass* pass)
{
    SetGraphPipeline(pass);
    if (!material || !pass) { return; }

    if (auto propertyBlock = material->GetMaterialParameterSet()) { SetShaderParameterSet(propertyBlock); }

    ApplyPushedShaderParameterSets();
}

void MRenderPassCmd::SetMaterial(const MMaterial* material, const MStringId& passName)
{
    if (!material) { return; }

    auto pass = material->GetTemplate()->GetPass(passName);

    SetMaterial(material, pass);
}

void MRenderPassCmd::UpdateBuffer(MBuffer* buffer, const MByte* data, const size_t& size)
{
    if (!buffer) { return; }

    if (buffer->m_stageType == MBuffer::MStageType::EWaitAllow)
    {
        buffer->DestroyBuffer(m_device);
        buffer->GenerateBuffer(m_device, data, size);
    }
    else if (buffer->m_stageType == MBuffer::MStageType::EWaitSync) { buffer->UploadBuffer(m_device, data, size); }
}
