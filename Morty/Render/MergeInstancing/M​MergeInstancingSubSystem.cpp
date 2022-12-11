#include "M​MergeInstancingSubSystem.h"
#include "Component/MRenderableMeshComponent.h"

#include "Utility/MFunction.h"

#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MNotifySystem.h"

MORTY_INTERFACE_IMPLEMENT(MMergeInstancingSubSystem, MISubSystem)

MMergeInstancingSubSystem::MMergeInstancingSubSystem()
	: MISubSystem()
	, m_pMergeMesh(nullptr)
{

}

MMergeInstancingSubSystem::~MMergeInstancingSubSystem()
{

}

void MMergeInstancingSubSystem::Initialize()
{
	Super::Initialize();

	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->RegisterNotify("MeshBatchChanged", M_CLASS_FUNCTION_BIND_1(MMergeInstancingSubSystem::OnBatchMeshChanged, this));
		pNotifySystem->RegisterNotify("TransformDirty", M_CLASS_FUNCTION_BIND_1(MMergeInstancingSubSystem::OnTransformDirty, this));
	}

	if (MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>())
	{
		m_pMergeMesh = pObjectSystem->CreateObject<MMergeInstancingMesh>();
	}

}

void MMergeInstancingSubSystem::Release()
{
	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->UnregisterNotify("MeshBatchChanged", M_CLASS_FUNCTION_BIND_1(MMergeInstancingSubSystem::OnBatchMeshChanged, this));
	}

	if (m_pMergeMesh)
	{
		m_pMergeMesh->DeleteLater();
		m_pMergeMesh = nullptr;
	}

	Super::Release();
}

void MMergeInstancingSubSystem::SceneTick(MScene* pScene, const float& fDelta)
{
}

void MMergeInstancingSubSystem::RegisterMaterial(MRenderableMeshComponent* pComponent)
{
	std::shared_ptr<MMaterial> pNewMaterial = pComponent->GetMaterial();

	auto&& findResult = m_tComponentMaterialTable.find(pComponent);
	if (findResult != m_tComponentMaterialTable.end())
	{
		std::shared_ptr<MMaterial> pOldMaterial = findResult->second;
		if (pNewMaterial == pOldMaterial)
		{
			return;
		}
		else
		{
			UnregisterMaterial(pComponent);
		}
	}

	if (pNewMaterial)
	{
		MMaterialBatchGroup* pMaterialBatchGroup = GetMaterialBatchGroup(pNewMaterial);
		pMaterialBatchGroup->vMeshComponent.insert(pComponent);

		m_tComponentMaterialTable[pComponent] = pNewMaterial;
	}
}

void MMergeInstancingSubSystem::UnregisterMaterial(MRenderableMeshComponent* pComponent)
{
	auto&& findResult = m_tComponentMaterialTable.find(pComponent);
	if (findResult != m_tComponentMaterialTable.end())
	{
		std::shared_ptr<MMaterial> pOldMaterial = findResult->second;
		
		if (pOldMaterial)
		{
			MMaterialBatchGroup* pMaterialBatchGroup = GetMaterialBatchGroup(pOldMaterial);
			pMaterialBatchGroup->vMeshComponent.erase(pComponent);

			if (pMaterialBatchGroup->vMeshComponent.empty())
			{
				delete pMaterialBatchGroup;
				m_tMaterialToBatchInstanceTable[pOldMaterial] = nullptr;
			}
		}
	}

}

void MMergeInstancingSubSystem::RegisterMesh(MRenderableMeshComponent* pComponent)
{
	MIMesh* pNewMesh = pComponent->GetMesh();

	auto&& findResult = m_tComponentMeshTable.find(pComponent);
	if (findResult != m_tComponentMeshTable.end())
	{
		MIMesh* pOldMesh = findResult->second;
		if (pOldMesh == pNewMesh)
		{
			return;
		}
		else
		{
			UnregisterMesh(pComponent);
		}
		
	}

	if (pNewMesh)
	{
		if (m_tMeshReferenceCount.find(pNewMesh) == m_tMeshReferenceCount.end())
		{
			m_tMeshReferenceCount[pNewMesh] = 1;
			m_pMergeMesh->RegisterMesh(pNewMesh);
		}
		else
		{
			m_tMeshReferenceCount[pNewMesh]++;
		}
		m_tComponentMeshTable[pComponent] = pNewMesh;
	}

}

void MMergeInstancingSubSystem::UnregisterMesh(MRenderableMeshComponent* pComponent)
{
	auto&& findResult = m_tComponentMeshTable.find(pComponent);
	if (findResult != m_tComponentMeshTable.end())
	{
		if (MIMesh* pOldMesh = findResult->second)
		{
			m_tMeshReferenceCount[pOldMesh]--;
			if (m_tMeshReferenceCount[pOldMesh] == 0)
			{
				m_tMeshReferenceCount.erase(pOldMesh);
				m_pMergeMesh->UnregisterMesh(pOldMesh);
			}
		}
		m_tComponentMeshTable.erase(pComponent);
	}
}


MMaterialBatchGroup* MMergeInstancingSubSystem::GetMaterialBatchGroup(std::shared_ptr<MMaterial> pMaterial)
{
	auto&& findResult = m_tMaterialToBatchInstanceTable.find(pMaterial);
	if (findResult != m_tMaterialToBatchInstanceTable.end())
	{
		return findResult->second;
	}

	MMaterialBatchGroup* pMaterialBatchGroup = m_tMaterialToBatchInstanceTable[pMaterial] = new MMaterialBatchGroup();


	pMaterialBatchGroup->pMaterial = pMaterial;


	return pMaterialBatchGroup;
}

const std::vector<MMergeInstancingMesh::MMeshClusterData>& MMergeInstancingSubSystem::GetMeshClusterGroup(MIMesh* pMesh)
{
	const MMergeInstancingMesh::MMeshMergeData&  meshMergeData = m_pMergeMesh->FindMesh(pMesh);

	return meshMergeData.vIndexData;
}

void MMergeInstancingSubSystem::OnBatchMeshChanged(MComponent* pSender)
{
	if (!pSender)
	{
		return;
	}

	if (MRenderableMeshComponent* pComponent = pSender->DynamicCast<MRenderableMeshComponent>())
	{
		if (pComponent->GetBatchInstanceEnable())
		{
			RegisterMesh(pComponent);
			RegisterMaterial(pComponent);
		}
		else
		{
			UnregisterMesh(pComponent);
			UnregisterMaterial(pComponent);
		}
	}
}

void MMergeInstancingSubSystem::OnTransformDirty(MComponent* pSender)
{
	
}
