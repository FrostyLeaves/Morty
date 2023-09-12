#include "MIndexedIndirectRenderable.h"

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

void MIndexedIndirectRenderable::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MIndexedIndirectRenderable::SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter)
{
	pMaterialFilter = pFilter;
}

void MIndexedIndirectRenderable::SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
{
	m_vFramePropertyAdapter = vAdapter;
}

void MIndexedIndirectRenderable::SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCullingAdapter)
{
	m_pCullingAdapter = pCullingAdapter;
}


const std::shared_ptr<MMaterial>& MIndexedIndirectRenderable::GetMaterial(const MMaterialCullingGroup& group) const
{
	const auto& pMaterial = group.pMaterial;
	return pMaterial;
}

void MIndexedIndirectRenderable::Render(MIRenderCommand* pCommand)
{
	if (!m_pScene)
	{
		MORTY_ASSERT(m_pScene);
		return;
	}
	

	MScene* pScene = m_pScene;
	const MMeshManager* pMeshManager = pScene->GetEngine()->FindGlobalObject<MMeshManager>();

	const MBuffer* drawIndirectBuffer = m_pCullingAdapter->GetDrawIndirectBuffer();
	const std::vector<MMaterialCullingGroup>& vCullingResult = m_pCullingAdapter->GetCullingInstanceGroup();

	if (!drawIndirectBuffer || drawIndirectBuffer->m_VkBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	for (const auto& group : vCullingResult)
	{
		if (group.nIndirectCount == 0)
		{
			continue;
		}

		const auto& pMaterial = GetMaterial(group);
		if (pMaterial == nullptr)
		{
			MORTY_ASSERT(pMaterial);
			continue;
		}

		if (pMaterialFilter && !pMaterialFilter->Filter(pMaterial))
		{
			continue;
		}

		pCommand->SetUseMaterial(pMaterial);

		for (auto& vPropertyBlock : m_vFramePropertyAdapter)
		{
			pCommand->SetShaderPropertyBlock(vPropertyBlock->GetPropertyBlock());
		}

		pCommand->SetShaderPropertyBlock(group.pMeshTransformProperty);

		pCommand->DrawIndexedIndirect(
			pMeshManager->GetVertexBuffer(),
			pMeshManager->GetIndexBuffer(),
			drawIndirectBuffer,
			group.nIndirectBeginIdx * sizeof(MDrawIndexedIndirectData),
			group.nIndirectCount
		);
	}

}
