#include "MIndirectIndexRenderable.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "Batch/MMaterialBatchGroup.h"


void MIndirectIndexRenderable::Render(MIRenderCommand* pCommand)
{
	const MBuffer* pIndirectBuffer = m_pBuffer;
	const MBuffer* pVertexBuffer = m_pMeshBuffer->GetVertexBuffer();
	const MBuffer* pIndexBuffer = m_pMeshBuffer->GetIndexBuffer();

	if (!pIndirectBuffer || pIndirectBuffer->m_VkBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	const auto& pMaterial = GetMaterial();
	if (pMaterial == nullptr)
	{
		MORTY_ASSERT(pMaterial);
		return;
	}

	pCommand->SetUseMaterial(pMaterial);

	for (auto& vPropertyBlock : m_vPropertyAdapter)
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
