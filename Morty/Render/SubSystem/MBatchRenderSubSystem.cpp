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

	std::map<std::weak_ptr<MMaterial>, MBatchInstanceData*> m_tMaterialToBatchInstanceTable;
	std::map<MRenderableMeshComponent*, MBatchInstanceData*> m_tComponentToBatchInstanceTable;

	MBatchInstanceData* pBatchInstanceData = nullptr;

	auto findResultByMaterial = m_tMaterialToBatchInstanceTable.find(pMaterial);
	if (findResultByMaterial == m_tMaterialToBatchInstanceTable.end())
	{
		pBatchInstanceData = new MBatchInstanceData();
		pBatchInstanceData->pMaterial = pMaterial;


		m_tMaterialToBatchInstanceTable[pMaterial] = pBatchInstanceData;
	}
	else
	{
		pBatchInstanceData = findResultByMaterial->second;
	}

	m_tComponentToBatchInstanceTable[pComponent] = pBatchInstanceData;


	MSharedMeshComponentGroup* pSharedGroup = nullptr;

	auto findResultByMesh = pBatchInstanceData->m_tMeshToGroup.find(pMesh);
	if (findResultByMesh == pBatchInstanceData->m_tMeshToGroup.end())
	{
		pSharedGroup = new MSharedMeshComponentGroup();
		pSharedGroup->pMesh = pMesh;

		pBatchInstanceData->m_tMeshToGroup[pMesh] = pSharedGroup;
	}
	else
	{
		pSharedGroup = findResultByMesh->second;
	}

	pBatchInstanceData->m_tComponentToGroup[pComponent] = pSharedGroup;
	pSharedGroup->m_tComponents.insert(pComponent);
}

void MBatchRenderSubSystem::UnregisterComponent(MRenderableMeshComponent* pComponent)
{
	auto findResultByComponent = m_tComponentToBatchInstanceTable.find(pComponent);
	if (findResultByComponent == m_tComponentToBatchInstanceTable.end())
	{
		return;
	}

	MBatchInstanceData* pBatchInstanceData = findResultByComponent->second;
	m_tComponentToBatchInstanceTable.erase(findResultByComponent);


	auto findResultFromSharedGroup = pBatchInstanceData->m_tComponentToGroup.find(pComponent);
	if (findResultFromSharedGroup != pBatchInstanceData->m_tComponentToGroup.end())
	{
		MSharedMeshComponentGroup* pSharedGroup = findResultFromSharedGroup->second;
		pBatchInstanceData->m_tComponentToGroup.erase(findResultFromSharedGroup);

		pSharedGroup->m_tComponents.erase(pComponent);


		if (pSharedGroup->m_tComponents.empty())
		{
			pBatchInstanceData->m_tMeshToGroup.erase(pSharedGroup->pMesh);
			delete pSharedGroup;
			pSharedGroup = nullptr;
		}
	}

	if (pBatchInstanceData->m_tComponentToGroup.empty())
	{
		m_tMaterialToBatchInstanceTable.erase(pBatchInstanceData->pMaterial);
		delete pBatchInstanceData;
		pBatchInstanceData = nullptr;
	}


}

void MBatchRenderSubSystem::OnBatchMeshChanged(MComponent* pSender)
{
}
