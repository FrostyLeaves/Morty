#include "MBatchRenderSubSystem.h"
#include "Component/MRenderableMeshComponent.h"

#include "Utility/MFunction.h"

#include "Engine/MEngine.h"
#include "System/MRenderSystem.h"
#include "System/MNotifySystem.h"

MORTY_INTERFACE_IMPLEMENT(MBatchRenderSubSystem, MISubSystem)

MBatchRenderSubSystem::MBatchRenderSubSystem()
	: MISubSystem()
{

}

MBatchRenderSubSystem::~MBatchRenderSubSystem()
{

}

void MBatchRenderSubSystem::Initialize()
{
	Super::Initialize();

	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->RegisterNotify("MeshBatchChanged", M_CLASS_FUNCTION_BIND_1(MBatchRenderSubSystem::OnBatchMeshChanged, this));
	}

}

void MBatchRenderSubSystem::Release()
{
	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->UnregisterNotify("MeshBatchChanged", M_CLASS_FUNCTION_BIND_1(MBatchRenderSubSystem::OnBatchMeshChanged, this));
	}

	Super::Release();
}

void MBatchRenderSubSystem::SceneTick(MScene* pScene, const float& fDelta)
{
}

void MBatchRenderSubSystem::RegisterComponent(MRenderableMeshComponent* pComponent)
{
	std::shared_ptr<MMaterial> pMaterial = pComponent->GetMaterial();
	MIMesh* pMesh = pComponent->GetMesh();
	if (nullptr == pMaterial)
	{
		return;
	}

	std::map<std::weak_ptr<MMaterial>, MMaterialBatchGroup*> m_tMaterialToBatchInstanceTable;
	std::map<MRenderableMeshComponent*, MMaterialBatchGroup*> m_tComponentToBatchInstanceTable;

	MMaterialBatchGroup* pMaterialBatchGroup = nullptr;

	auto findResultByMaterial = m_tMaterialToBatchInstanceTable.find(pMaterial);
	if (findResultByMaterial == m_tMaterialToBatchInstanceTable.end())
	{
		pMaterialBatchGroup = new MMaterialBatchGroup();
		pMaterialBatchGroup->pMaterial = pMaterial;


		m_tMaterialToBatchInstanceTable[pMaterial] = pMaterialBatchGroup;
	}
	else
	{
		pMaterialBatchGroup = findResultByMaterial->second;
	}

	m_tComponentToBatchInstanceTable[pComponent] = pMaterialBatchGroup;


	MSharedMeshData* pSharedMesh = nullptr;

	auto findResultByMesh = pMaterialBatchGroup->m_tSharedMesh.find(pMesh);
	if (findResultByMesh == pMaterialBatchGroup->m_tSharedMesh.end())
	{
		pSharedMesh = new MSharedMeshData();
		pSharedMesh->pMesh = pMesh;

		pMaterialBatchGroup->m_tSharedMesh[pMesh] = pSharedMesh;
	}
	else
	{
		pSharedMesh = findResultByMesh->second;
	}

	pMaterialBatchGroup->m_tComponentToSharedMesh[pComponent] = pSharedMesh;
	pSharedMesh->m_tComponents.insert(pComponent);
}

void MBatchRenderSubSystem::UnregisterComponent(MRenderableMeshComponent* pComponent)
{
	auto findResultByComponent = m_tComponentToBatchInstanceTable.find(pComponent);
	if (findResultByComponent == m_tComponentToBatchInstanceTable.end())
	{
		return;
	}

	MMaterialBatchGroup* pMaterialBatchGroup = findResultByComponent->second;
	m_tComponentToBatchInstanceTable.erase(findResultByComponent);


	auto findResultFromSharedGroup = pMaterialBatchGroup->m_tComponentToSharedMesh.find(pComponent);
	if (findResultFromSharedGroup != pMaterialBatchGroup->m_tComponentToSharedMesh.end())
	{
		MSharedMeshData* pSharedMesh = findResultFromSharedGroup->second;
		pMaterialBatchGroup->m_tComponentToSharedMesh.erase(findResultFromSharedGroup);

		pSharedMesh->m_tComponents.erase(pComponent);


		if (pSharedMesh->m_tComponents.empty())
		{
			pMaterialBatchGroup->m_tSharedMesh.erase(pSharedMesh->pMesh);
			delete pSharedMesh;
			pSharedMesh = nullptr;
		}
	}

	if (pMaterialBatchGroup->m_tComponentToSharedMesh.empty())
	{
		m_tMaterialToBatchInstanceTable.erase(pMaterialBatchGroup->pMaterial);
		delete pMaterialBatchGroup;
		pMaterialBatchGroup = nullptr;
	}


}

void MBatchRenderSubSystem::OnBatchMeshChanged(MComponent* pSender)
{
	if (!pSender)
	{
		return;
	}

	if (MRenderableMeshComponent* pComponent = pSender->DynamicCast<MRenderableMeshComponent>())
	{
		UnregisterComponent(pComponent);

		if (pComponent->GetBatchInstanceEnable())
		{
			RegisterComponent(pComponent);
		}
	}
}
