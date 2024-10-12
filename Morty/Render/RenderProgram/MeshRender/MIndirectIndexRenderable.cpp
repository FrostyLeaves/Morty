#include "MIndirectIndexRenderable.h"

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

void MIndirectIndexRenderable::Render(MIRenderCommand* pCommand)
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

    pCommand->SetUseMaterial(pMaterial);

    for (auto& vPropertyBlock: m_propertyAdapter)
    {
        pCommand->SetShaderPropertyBlock(vPropertyBlock->GetPropertyBlock());
    }

    pCommand->DrawIndexedIndirect(
            pVertexBuffer,
            pIndexBuffer,
            pIndirectBuffer,
            0,
            pIndirectBuffer->GetSize() / sizeof(MDrawIndexedIndirectData)
    );
}
