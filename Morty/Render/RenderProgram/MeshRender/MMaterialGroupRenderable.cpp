#include "MMaterialGroupRenderable.h"

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
#include "Culling/MInstanceCulling.h"

void MMaterialGroupRenderable::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MMaterialGroupRenderable::SetMaterialFilter(std::shared_ptr<IMaterialFilter> pFilter)
{
	m_pMaterialFilter = pFilter;
}

void MMaterialGroupRenderable::SetInstanceFilter(std::shared_ptr<IMeshInstanceFilter> pFilter)
{
	m_pInstanceFilter = pFilter;
}

void MMaterialGroupRenderable::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MMaterialGroupRenderable::SetRenderableMaterialGroup(const std::vector<MRenderableMaterialGroup*>& vRenderGroup)
{
	m_vRenderGroup = vRenderGroup;
}

void MMaterialGroupRenderable::Render(MIRenderCommand* pCommand)
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

	const auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();

	MScene* pScene = m_pScene;
	const MMeshManager* pMeshManager = pScene->GetEngine()->FindGlobalObject<MMeshManager>();

	for (const auto& pMaterialGroup : m_vRenderGroup)
	{
		const auto pMaterial = pMaterialGroup->GetMaterial();

		if (m_pMaterialFilter && !m_pMaterialFilter->Filter(pMaterial))
		{
			continue;
		}

		pCommand->SetUseMaterial(pMaterial);
		for (const auto& property : vPropertyBlock)
		{
			pCommand->SetShaderParamSet(property);
		}

		const auto& vMeshGroup = pMaterialGroup->GetInstanceBatchGroup();
		for (const auto& pMeshGroup : vMeshGroup)
		{
			pCommand->SetShaderParamSet(pMeshGroup->GetMeshProperty());

			auto func = [=](const MRenderableMeshInstance& meshInstance, size_t)
			{
				if (!meshInstance.bVisible)
				{
					return;
				}

				if (m_pInstanceFilter && !m_pInstanceFilter->Filter(&meshInstance))
				{
					return;
				}

				const MMeshManager::MMeshData& meshData = pMeshManager->FindMesh(meshInstance.pMesh);

				pCommand->DrawMesh(
					pMeshManager->GetVertexBuffer(),
					pMeshManager->GetIndexBuffer(),
					0,
					meshData.indexInfo.begin,
					meshData.indexInfo.size);
			};

			pMeshGroup->InstanceExecute(func);

		}
	}
}
