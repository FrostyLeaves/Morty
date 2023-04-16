#include "MStaticMeshRender.h"

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
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"

#include "MergeInstancing/MRenderableMeshGroup.h"

void MStaticMeshRender::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MStaticMeshRender::SetRenderableFilter(std::shared_ptr<IRenderableFilter> pRenderableFilter)
{
	m_pRenderableFilter = pRenderableFilter;
}

void MStaticMeshRender::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MStaticMeshRender::SetRenderableMaterialGroup(const std::vector<MRenderableMaterialGroup*>& vRenderGroup)
{
	m_vRenderGroup = vRenderGroup;
}

void MStaticMeshRender::Render(MIRenderCommand* pCommand)
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

	auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();

	MScene* pScene = m_pScene;
	MMeshManager* pMeshManager = pScene->GetEngine()->FindGlobalObject<MMeshManager>();

	for (const auto& pRenderableMaterialGroup : m_vRenderGroup)
	{
		auto pMaterial = pRenderableMaterialGroup->GetMaterial();

		pCommand->SetUseMaterial(pMaterial);
		for (const auto& property : vPropertyBlock)
		{
			pCommand->SetShaderParamSet(property);
		}

		const auto& vMeshGroup = pRenderableMaterialGroup->GetRenderableMeshGroup();
		for (const auto& pMeshGroup : vMeshGroup)
		{
			pCommand->SetShaderParamSet(pMeshGroup->GetMeshProperty());

			const auto& vMeshInstance = pMeshGroup->GetMeshInstance();
			MORTY_ASSERT(vMeshInstance.size() == 1);

			for (const auto& meshInstance : vMeshInstance)
			{
				if (m_pRenderableFilter && !m_pRenderableFilter->Filter(&meshInstance))
				{
					continue;
				}

			    const MMeshManager::MMeshData& meshData = pMeshManager->FindMesh(meshInstance.pMesh);

			    pCommand->DrawMesh(
				    pMeshManager->GetVertexBuffer(),
				    pMeshManager->GetIndexBuffer(),
				    0,
				    meshData.indexInfo.begin,
				    meshData.indexInfo.size);
			}
		}
	}

	/*
	if (m_pGPUCullingRenderWork)
	{
		const MBuffer* pDrawIndirectBuffer = m_pGPUCullingRenderWork->GetDrawIndirectBuffer();
		const std::vector<MMaterialCullingGroup>& vCullingInstanceGroup = m_pGPUCullingRenderWork->GetCullingInstanceGroup();
		for (const MMaterialCullingGroup& group : vCullingInstanceGroup)
		{
			pCommand->SetUseMaterial(group.pMaterial);
			pCommand->SetShaderParamSet(m_forwardFramePropertyBlock.GetShaderPropertyBlock());

			//Binding MVP
			pCommand->SetShaderParamSet(group.pMeshTransformProperty);

			pCommand->DrawIndexedIndirect(group.pVertexBuffer, group.pIndexBuffer, pDrawIndirectBuffer, group.nClusterBeginIdx * sizeof(MDrawIndexedIndirectData), group.nClusterCount);
		}
	}
	*/

}
