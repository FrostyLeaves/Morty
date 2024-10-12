#include "MCullingResultRenderable.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Mesh/MVertex.h"

#include "Batch/MMaterialBatchGroup.h"
#include "Mesh/MMeshManager.h"
#include "Utility/MBounds.h"

using namespace morty;

void MCullingResultRenderable::SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter)
{
    pMaterialFilter = pFilter;
}

void MCullingResultRenderable::SetPropertyBlockAdapter(
        const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter
)
{
    m_framePropertyAdapter = vAdapter;
}

void MCullingResultRenderable::SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCullingAdapter)
{
    m_cullingAdapter = pCullingAdapter;
}

std::shared_ptr<MMaterial> MCullingResultRenderable::GetMaterial(const MMaterialCullingGroup& group) const
{
    const auto& pMaterial = group.pMaterial;
    return pMaterial;
}

void MCullingResultRenderable::Render(MIRenderCommand* pCommand)
{
    if (!m_meshBuffer)
    {
        MORTY_ASSERT(m_meshBuffer);
        return;
    }

    const MBuffer*                            pVertexBuffer   = m_meshBuffer->GetVertexBuffer();
    const MBuffer*                            pIndexBuffer    = m_meshBuffer->GetIndexBuffer();
    const MBuffer*                            pIndirectBuffer = m_cullingAdapter->GetDrawIndirectBuffer();
    const std::vector<MMaterialCullingGroup>& vCullingResult  = m_cullingAdapter->GetCullingInstanceGroup();

    if (!pIndirectBuffer || pIndirectBuffer->m_vkBuffer == VK_NULL_HANDLE) { return; }

    for (const auto& group: vCullingResult)
    {
        if (group.nIndirectCount == 0) { continue; }

        if (pMaterialFilter && !pMaterialFilter->Filter(group.pMaterial)) { continue; }

        const auto& pMaterial = GetMaterial(group);
        if (pMaterial == nullptr) { continue; }

        pCommand->SetGraphPipeline(pMaterial);

        for (auto& vPropertyBlock: m_framePropertyAdapter)
        {
            pCommand->SetShaderPropertyBlock(vPropertyBlock->GetPropertyBlock());
        }

        pCommand->SetShaderPropertyBlock(group.pMeshTransformProperty);
        pCommand->SetShaderPropertyBlock(group.pMaterial->GetMaterialPropertyBlock());

        pCommand->DrawIndexedIndirect(
                pVertexBuffer,
                pIndexBuffer,
                pIndirectBuffer,
                group.nIndirectBeginIdx * sizeof(MDrawIndexedIndirectData),
                group.nIndirectCount
        );
    }
}
