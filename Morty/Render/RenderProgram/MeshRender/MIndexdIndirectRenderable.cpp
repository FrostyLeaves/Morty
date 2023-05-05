#include "MIndexdIndirectRenderable.h"

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
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"

#include "MergeInstancing/MRenderableMeshGroup.h"

void MIndexdIndirectRenderable::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MIndexdIndirectRenderable::SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter)
{
	pMaterialFilter = pFilter;
}

void MIndexdIndirectRenderable::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MIndexdIndirectRenderable::SetInstanceCulling(const std::shared_ptr<MInstanceCulling>& pCullingAdapter)
{
	m_pCullingAdapter = pCullingAdapter;
}

void MIndexdIndirectRenderable::Render(MIRenderCommand* pCommand)
{
	if (!m_pScene)
	{
		MORTY_ASSERT(m_pScene);
		return;
	}

	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
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

		const auto& pMaterial = group.pMaterial;
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

		const auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
		for (const auto& property : vPropertyBlock)
		{
			pCommand->SetShaderParamSet(property);
		}

		pCommand->SetShaderParamSet(group.pMeshTransformProperty);

		pCommand->DrawIndexedIndirect(
			pMeshManager->GetVertexBuffer(),
			pMeshManager->GetIndexBuffer(),
			drawIndirectBuffer,
			group.nIndirectBeginIdx * sizeof(MDrawIndexedIndirectData),
			group.nIndirectCount
		);
	}

}

void MComputeDispatcherRender::SetComputeDispatcher(const std::shared_ptr<IComputeDispatcherAdapter>& pComputeDispatcher)
{
	m_pComputeDispatcher = pComputeDispatcher;
}

void MComputeDispatcherRender::Render(MIRenderCommand* pCommand)
{
	if (!m_pComputeDispatcher)
	{
		MORTY_ASSERT(m_pComputeDispatcher);
		return;
	}

	MComputeDispatcher* pComputeDispatcher = m_pComputeDispatcher->GetComputeDispatcher();
	if (!pComputeDispatcher)
	{
		MORTY_ASSERT(pComputeDispatcher);
		return;
	}

	const auto vComputeGroup = m_pComputeDispatcher->GetComputeGroup();
	auto vBarrierBuffer = m_pComputeDispatcher->GetBarrierBuffer();

	const size_t nGroupX = vComputeGroup[0];
	const size_t nGroupY = vComputeGroup[1];
	const size_t nGroupZ = vComputeGroup[2];

	pCommand->AddGraphToComputeBarrier({ vBarrierBuffer });
	pCommand->DispatchComputeJob(pComputeDispatcher, nGroupX, nGroupY, nGroupZ);
	pCommand->AddComputeToGraphBarrier({ vBarrierBuffer });
}
