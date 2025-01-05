#include "MRenderPassCmd.h"
#include "Material/MMaterial.h"
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
            {.vertexBuffer = pVertexBuffer,
             .indexBuffer  = pIndexBuffer,
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
            .vertexBuffer   = pVertexBuffer,
            .indexBuffer    = pIndexBuffer,
            .commandsBuffer = pCommandsBuffer,
            .offset         = offset,
            .count          = count,
    });
}

void MRenderPassCmd::SetGraphPipeline(const MGraphicsPipeline* pipeline, size_t subPassIdx)
{
    m_commandQueue.emplace_back(new MSetGraphPipelineCmd{
            .pipeline   = pipeline,
            .subPassIdx = subPassIdx,
    });
}

void MRenderPassCmd::SetShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& block)
{
    SetShaderPropertyBlock(block.get());
}

void MRenderPassCmd::SetShaderPropertyBlock(MShaderPropertyBlock* pPropertyBlock)
{
    m_commandQueue.emplace_back(new MSetShaderPropertyBlockCmd{
            .pipeline = m_usingPipeline,
            .property = pPropertyBlock,
    });
}

void MRenderPassCmd::PushShaderPropertyBlock(MShaderPropertyBlock* pPropertyBlock)
{
    m_propertyBlockStack.emplace_back(pPropertyBlock);
}

void MRenderPassCmd::PopShaderPropertyBlock() { m_propertyBlockStack.pop_back(); }

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
    m_commandQueue.emplace_back(
            new MSetScissorCmd{.x = viewport.x, .y = viewport.y, .width = viewport.width, .height = viewport.height}
    );
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
    if (!mesh) return;

    MBuffer* pVertexBuffer = mesh->GetVertexBuffer();
    MBuffer* pIndexBuffer  = mesh->GetIndexBuffer();

    if (!pVertexBuffer || !pIndexBuffer) { return; }

    UpdateBuffer(pVertexBuffer, mesh->GetVerticesVector().data(), mesh->GetVerticesVector().size());
    UpdateBuffer(pIndexBuffer, mesh->GetIndicesVector().data(), mesh->GetIndicesVector().size());

    DrawMesh(pVertexBuffer, pIndexBuffer, nVertexOffset, nIndexOffset, nIndexCount);
}

void MRenderPassCmd::DrawMesh(MIMesh* mesh) { DrawMesh(mesh, 0, mesh->GetIndicesNum(), 0); }


void MRenderPassCmd::SetGraphPipeline(const MMaterialTemplate* materialTemplate)
{
    const auto pPipeline = m_device->FindOrCreateGraphicsPipeline(materialTemplate, m_renderPass);
    MORTY_ASSERT(nullptr != pPipeline);
    if (m_usingPipeline == pPipeline.get()) { return; }
    m_usingPipeline = pPipeline.get();

    const auto pGraphicsPipeline = std::dynamic_pointer_cast<MGraphicsPipeline>(pPipeline);

    SetGraphPipeline(pGraphicsPipeline.get(), m_subPassIdx);
    SetShadingRate(materialTemplate->GetShadingRate(), {MEShadingRateCombinerOp::Max, MEShadingRateCombinerOp::Max});
}

void MRenderPassCmd::SetMaterial(const MMaterial* material)
{
    const auto& pMaterialTemplate = material->GetMaterialTemplate();
    if (nullptr == pMaterialTemplate) { return; }

    SetGraphPipeline(pMaterialTemplate.get());

    auto propertyBlock = pMaterialTemplate->GetMaterialPropertyBlock().get();
    SetShaderPropertyBlock(propertyBlock);

    for (const auto& pPushedProperty: m_propertyBlockStack) { SetShaderPropertyBlock(pPushedProperty); }
}

void MRenderPassCmd::SetMaterial(const MMaterialTemplate* pMaterialTemplate)
{
    if (nullptr == pMaterialTemplate) { return; }

    SetGraphPipeline(pMaterialTemplate);

    auto propertyBlock = pMaterialTemplate->GetMaterialPropertyBlock().get();
    SetShaderPropertyBlock(propertyBlock);

    for (const auto& pPushedProperty: m_propertyBlockStack) { SetShaderPropertyBlock(pPushedProperty); }
}

void MRenderPassCmd::UpdateBuffer(MBuffer* pBuffer, const MByte* data, const size_t& size)
{
    if (!pBuffer) { return; }

    if (pBuffer->m_stageType == MBuffer::MStageType::EWaitAllow)
    {
        pBuffer->DestroyBuffer(m_device);
        pBuffer->GenerateBuffer(m_device, data, size);
    }
    else if (pBuffer->m_stageType == MBuffer::MStageType::EWaitSync) { pBuffer->UploadBuffer(m_device, data, size); }
}
