#include "MRenderableMeshManager.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "Utility/MFunction.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"

#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MNotifySystem.h"

#include "MRenderNotify.h"
#include "Module/MCoreNotify.h"

#include "Mesh/MMeshManager.h"

MORTY_INTERFACE_IMPLEMENT(MRenderableMeshManager, IManager)

void MRenderableMeshManager::Initialize()
{
	Super::Initialize();

	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnTransformChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMaterialChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMeshChanged, this));
	}
}

void MRenderableMeshManager::Release()
{
	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnTransformChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMaterialChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMeshChanged, this));
	}

	Super::Release();
}

void MRenderableMeshManager::SceneTick(MScene* pScene, const float& fDelta)
{
	for(auto pComponent : m_tWaitUpdateRenderGroupComponent)
	{
		RemoveComponentFromGroup(pComponent);
		AddComponentToGroup(pComponent);
	}
	m_tWaitUpdateRenderGroupComponent.clear();

	for (auto pComponent : m_tWaitUpdateMeshComponent)
	{
		BindMesh(pComponent, pComponent->GetMesh());
	}
	m_tWaitUpdateMeshComponent.clear();

	for (auto pComponent : m_tWaitUpdateTransformComponent)
	{
		UpdateTransform(pComponent);
	}
	m_tWaitUpdateTransformComponent.clear();
}

void MRenderableMeshManager::OnTransformChanged(MComponent* pComponent)
{
	if (MRenderableMeshComponent* pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>())
	{
		AddQueueUpdateTransform(pMeshComponent);
	}
}

void MRenderableMeshManager::OnMaterialChanged(MComponent* pComponent)
{
	if (MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddQueueUpdateRenderGroup(pMeshComponent);
	}
}

void MRenderableMeshManager::OnMeshChanged(MComponent* pComponent)
{
	if (MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		AddQueueUpdateMesh(pMeshComponent);
		AddQueueUpdateRenderGroup(pMeshComponent);
	}
}

void MRenderableMeshManager::AddQueueUpdateTransform(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateTransformComponent.insert(pComponent);
}

void MRenderableMeshManager::AddQueueUpdateMesh(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateMeshComponent.insert(pComponent);
}

void MRenderableMeshManager::AddQueueUpdateRenderGroup(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateRenderGroupComponent.insert(pComponent);
}

void MRenderableMeshManager::RemoveComponent(MRenderableMeshComponent* pComponent)
{
	m_tWaitUpdateMeshComponent.erase(pComponent);
	m_tWaitUpdateTransformComponent.erase(pComponent);
	m_tWaitUpdateRenderGroupComponent.erase(pComponent);

	BindMesh(pComponent, nullptr);
	RemoveComponentFromGroup(pComponent);
}

std::vector<MRenderableMaterialGroup*> MRenderableMeshManager::FindGroupFromMaterialType(MEMaterialType eType) const
{
	std::vector<MRenderableMaterialGroup*> vRenderableGroup;
	const auto& tMaterialGroup = GetRenderableMaterialGroup();
	for (auto pr : tMaterialGroup)
	{
		auto pMaterial = pr.first;
		if (pMaterial->GetMaterialType() == eType)
		{
			vRenderableGroup.push_back(pr.second);
		}
	}

	return vRenderableGroup;
}

void MRenderableMeshManager::AddComponentToGroup(MRenderableMeshComponent* pComponent)
{
	const auto pMaterial = pComponent->GetMaterial();
	if (nullptr == pMaterial)
	{
		return;
	}

	const auto findResult = m_tRenderableMaterialGroup.find(pMaterial);
	if (findResult != m_tRenderableMaterialGroup.end())
	{
		findResult->second->AddMeshInstance(pComponent);
		m_tComponentTable[pComponent] = findResult->second;
		return;
	}

	auto pRenderableGroup = new MRenderableMaterialGroup();
	m_tRenderableMaterialGroup[pMaterial] = pRenderableGroup;
	pRenderableGroup->Initialize(GetEngine(), pMaterial);

	pRenderableGroup->AddMeshInstance(pComponent);
	m_tComponentTable[pComponent] = pRenderableGroup;
}

void MRenderableMeshManager::RemoveComponentFromGroup(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	MRenderableMaterialGroup* pMaterialGroup = findResult->second;

	pMaterialGroup->RemoveMeshInstance(pComponent);
	if (pMaterialGroup->IsEmpty())
	{
		m_tRenderableMaterialGroup.erase(pMaterialGroup->GetMaterial());
		pMaterialGroup->Release(GetEngine());
		delete pMaterialGroup;
	}

	m_tComponentTable.erase(findResult);
}

void MRenderableMeshManager::UpdateTransform(MSceneComponent* pComponent)
{
	auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>();
	UpdateTransform(pMeshComponent);
}

void MRenderableMeshManager::UpdateTransform(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	MRenderableMaterialGroup* pMaterialGroup = findResult->second;
	if (!pMaterialGroup)
	{
		MORTY_ASSERT(pMaterialGroup);
		return;
	}

	pMaterialGroup->UpdateTransform(pComponent);
}

void MRenderableMeshManager::BindMesh(MRenderableMeshComponent* pComponent, MIMesh* pMesh)
{
	MMeshReferenceMap* pOriginReference = nullptr;
	MMeshReferenceMap* pNewReference = nullptr;

	auto findResult = m_tMeshReferenceComponentTable.find(pComponent);
	if (findResult != m_tMeshReferenceComponentTable.end())
	{
		pOriginReference = findResult->second;
	}

	if (pMesh)
	{
		auto findMesh = m_tMeshReferenceTable.find(pMesh);
		if (findMesh == m_tMeshReferenceTable.end())
		{
			pNewReference = new MMeshReferenceMap();
			pNewReference->pMesh = pMesh;
			m_tMeshReferenceTable[pMesh] = pNewReference;

			MMeshManager* pMeshManager = GetScene()->GetEngine()->FindGlobalObject<MMeshManager>();
			if (!pMeshManager->HasMesh(pMesh))
			{
				pMeshManager->RegisterMesh(pMesh);
			}
		}
		else
		{
			pNewReference = findMesh->second;
		}
	}

	if (pOriginReference == pNewReference)
	{
		return;
	}

	if (pOriginReference)
	{
		pOriginReference->tComponents.erase(pComponent);
		if (pOriginReference->tComponents.empty())
		{
			MMeshManager* pMeshManager = GetScene()->GetEngine()->FindGlobalObject<MMeshManager>();
			pMeshManager->UnregisterMesh(pOriginReference->pMesh);

			m_tMeshReferenceTable.erase(pOriginReference->pMesh);
		}
	}

	if (pNewReference)
	{
		pNewReference->tComponents.insert(pComponent);
	}
}
