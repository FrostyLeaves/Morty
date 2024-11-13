#include "MIndirectIndexRenderable.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Batch/MMaterialBatchGroup.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"

using namespace morty;

void MIndirectIndexRenderable::Render(MRenderPassCmd* pCommand)
{
    const MBuffer* pIndirectBuffer = m_buffer;
    const MBuffer* pVertexBuffer   = m_meshBuffer->GetVertexBuffer();
    const MBuffer* pIndexBuffer    = m_meshBuffer->GetIndexBuffer();

    if (!pIndirectBuffer || pIndirectBuffer->m_vkBuffer == VK_NULL_HANDLE) { return; }

    const auto& pMaterial = GetMaterial();
    if (pMaterial == nullptr)
    {
        MORTY_ASSERT(pMaterial);
        return;
    }

    pCommand->SetMaterial(pMaterial.get());

    for (auto& vPropertyBlock: m_propertyAdapter)
    {
        pCommand->SetShaderPropertyBlock(vPropertyBlock->GetPropertyBlock().get());
    }

    pCommand->DrawIndexedIndirect(
            pVertexBuffer,
            pIndexBuffer,
            pIndirectBuffer,
            0,
            pIndirectBuffer->GetSize() / sizeof(MDrawIndexedIndirectData)
    );
}
