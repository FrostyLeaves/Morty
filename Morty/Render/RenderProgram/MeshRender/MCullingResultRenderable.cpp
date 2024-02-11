#include "MCullingResultRenderable.h"

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

using namespace morty;

void MCullingResultRenderable::SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter)
{
	pMaterialFilter = pFilter;
}

void MCullingResultRenderable::SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
{
	m_vFramePropertyAdapter = vAdapter;
}

void MCullingResultRenderable::SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCullingAdapter)
{
	m_pCullingAdapter = pCullingAdapter;
}

std::shared_ptr<MMaterial> MCullingResultRenderable::GetMaterial(const MMaterialCullingGroup& group) const
{
	const auto& pMaterial = group.pMaterial;
	return pMaterial;
}

void MCullingResultRenderable::Render(MIRenderCommand* pCommand)
{
	if (!m_pMeshBuffer)
	{
		MORTY_ASSERT(m_pMeshBuffer);
		return;
	}
	
	const MBuffer* pVertexBuffer = m_pMeshBuffer->GetVertexBuffer();
	const MBuffer* pIndexBuffer = m_pMeshBuffer->GetIndexBuffer();
	const MBuffer* pIndirectBuffer = m_pCullingAdapter->GetDrawIndirectBuffer();
	const std::vector<MMaterialCullingGroup>& vCullingResult = m_pCullingAdapter->GetCullingInstanceGroup();

	if (!pIndirectBuffer || pIndirectBuffer->m_VkBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	for (const auto& group : vCullingResult)
	{
		if (group.nIndirectCount == 0)
		{
			continue;
		}

		if (pMaterialFilter && !pMaterialFilter->Filter(group.pMaterial))
		{
			continue;
		}

		const auto& pMaterial = GetMaterial(group);
		if (pMaterial == nullptr)
		{
			continue;
		}

		pCommand->SetGraphPipeline(pMaterial);

		for (auto& vPropertyBlock : m_vFramePropertyAdapter)
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
