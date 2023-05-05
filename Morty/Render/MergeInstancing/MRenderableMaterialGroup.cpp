#include "MRenderableMeshGroup.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Material/MShaderParamSet.h"
#include "System/MRenderSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "InstanceBatch/MNoneBatchGroup.h"
#include "InstanceBatch/MUniformBatchGroup.h"
#include "InstanceBatch/MStorageBatchGroup.h"
#include "Render/MVertex.h"

class MORTY_API MMaterialBatchUtil
{
public:
	enum MORTY_API TransformType
	{
		ENone = 0,
		EUniformArray = 1,
		EStorageBuffer = 2
	};


	static TransformType GetMaterialBatchTransformType(MMaterial* pMaterial);

	static MInstanceBatchGroup* CreateBatchGroup(MMaterial* pMaterial);
};


void MRenderableMaterialGroup::Initialize(MEngine* pEngine, std::shared_ptr<MMaterial> pMaterial)
{
	m_pEngine = pEngine;
	m_pMaterial = pMaterial;
}

void MRenderableMaterialGroup::Release(MEngine* pEngine)
{
	for(auto pMeshGroup : m_vRenderableMeshGroup)
	{
		pMeshGroup->Release(pEngine);
		delete pMeshGroup;
	}
	m_vRenderableMeshGroup.clear();
}

void MRenderableMaterialGroup::AddMeshInstance(MRenderableMeshComponent* pComponent)
{
	MSceneComponent* pSceneComponent = pComponent->GetEntity()->GetComponent<MSceneComponent>();
	if (!pComponent)
	{
		MORTY_ASSERT(false);
		return;
	}

	if (!pSceneComponent)
	{
		MORTY_ASSERT(false);
		return;
	}

	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult != m_tMeshComponentTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nMeshGroupIdx = 0;
	MInstanceBatchGroup* pMeshGroup = nullptr;
	for (size_t nIdx = 0; nIdx < m_vRenderableMeshGroup.size(); ++nIdx)
	{
		MInstanceBatchGroup* pCurrentGroup = m_vRenderableMeshGroup[nIdx];
		if (pCurrentGroup->CanAddMeshInstance())
		{
			nMeshGroupIdx = nIdx;
			pMeshGroup = pCurrentGroup;
			break;
		}
	}
	if (pMeshGroup == nullptr)
	{
		pMeshGroup = MMaterialBatchUtil::CreateBatchGroup(m_pMaterial.get());
		pMeshGroup->Initialize(m_pEngine, m_pMaterial);
		m_vRenderableMeshGroup.push_back(pMeshGroup);
		nMeshGroupIdx = m_vRenderableMeshGroup.size() - 1;
	}

	if (!pMeshGroup->CanAddMeshInstance())
	{
		MORTY_ASSERT(false);
		return;
	}

	pMeshGroup->AddMeshInstance(pComponent);
	m_tMeshComponentTable[pComponent] = nMeshGroupIdx;

}

void MRenderableMaterialGroup::RemoveMeshInstance(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if(findResult == m_tMeshComponentTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	size_t nIdx = findResult->second;
	m_tMeshComponentTable.erase(findResult);

	if(nIdx >= m_vRenderableMeshGroup.size())
	{
		MORTY_ASSERT(nIdx < m_vRenderableMeshGroup.size());
		return;
	}

	m_vRenderableMeshGroup[nIdx]->RemoveMeshInstance(pComponent);
}

void MRenderableMaterialGroup::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		return;
	}

	size_t nIdx = findResult->second;
	m_vRenderableMeshGroup[nIdx]->UpdateTransform(pComponent);
}

void MRenderableMaterialGroup::UpdateMesh(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		return;
	}

	size_t nIdx = findResult->second;
	m_vRenderableMeshGroup[nIdx]->UpdateMesh(pComponent);
}

void MRenderableMaterialGroup::UpdateVisible(MRenderableMeshComponent* pComponent, bool bVisible)
{
	const auto findResult = m_tMeshComponentTable.find(pComponent);
	if (findResult == m_tMeshComponentTable.end())
	{
		return;
	}

	size_t nIdx = findResult->second;
	m_vRenderableMeshGroup[nIdx]->UpdateVisible(pComponent, bVisible);
}

bool MRenderableMaterialGroup::IsEmpty() const
{
	return m_tMeshComponentTable.empty();
}

MMaterialBatchUtil::TransformType MMaterialBatchUtil::GetMaterialBatchTransformType(MMaterial* pMaterial)
{
	if (!pMaterial)
	{
		return TransformType::ENone;
	}

	if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM))
	{
		return TransformType::EUniformArray;
	}
	if (pMaterial->GetShaderMacro().HasMacro(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE))
	{
		return TransformType::EStorageBuffer;
	}

	return TransformType::ENone;
}

MInstanceBatchGroup* MMaterialBatchUtil::CreateBatchGroup(MMaterial* pMaterial)
{
	auto type = GetMaterialBatchTransformType(pMaterial);

	if (TransformType::EUniformArray == type)
	{
		return new MUniformBatchGroup();
	}
	if (TransformType::EStorageBuffer == type)
	{
		return new MStorageBatchGroup();
	}
	if (TransformType::ENone == type)
	{
		return new MNoneBatchGroup();
	}

	MORTY_ASSERT(false);
	return nullptr;
}
