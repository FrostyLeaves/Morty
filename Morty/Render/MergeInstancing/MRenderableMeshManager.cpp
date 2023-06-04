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
#include "TaskGraph/MTaskGraph.h"

MORTY_INTERFACE_IMPLEMENT(MRenderableMeshManager, IManager)

void MRenderableMeshManager::Initialize()
{
	Super::Initialize();

	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_VISIBLE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnVisibleChanged, this));
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnTransformChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMaterialChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMeshChanged, this));
	}

	if (m_pUpdateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>("RenderMeshManagerUpdate"))
	{
		m_pUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::RenderUpdate, this));
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);
	}
}

void MRenderableMeshManager::Release()
{
	if (MNotifySystem* pNotifySystem = GetEngine()->FindSystem<MNotifySystem>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_VISIBLE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnVisibleChanged, this));
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnTransformChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMaterialChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MRenderableMeshManager::OnMeshChanged, this));
	}

	Clean();

	Super::Release();
}

void MRenderableMeshManager::RenderUpdate(MTaskNode* pNode)
{
	for (auto& [material, group] : m_tRenderableMaterialGroup)
	{
		if (!group->tWaitRemoveComponent.empty())
		{
			for (const auto& key : group->tWaitRemoveComponent)
			{
				group->materialGroup.RemoveMeshInstance(key);
			}
			group->tWaitRemoveComponent.clear();
		}

		if (!group->tWaitAddComponent.empty())
		{
			for (auto& [key, proxy] : group->tWaitAddComponent)
			{
				group->materialGroup.AddMeshInstance(key, proxy);
			}
			group->tWaitAddComponent.clear();
		}

		if (!group->tWaitUpdateComponent.empty())
		{
			for (auto& [key, proxy] : group->tWaitUpdateComponent)
			{
				group->materialGroup.UpdateMeshInstance(key, proxy);
			}
			group->tWaitUpdateComponent.clear();
		}

		if (group->materialGroup.IsEmpty())
		{
			m_tRenderableMaterialGroup.erase(group->materialGroup.GetMaterial());
			group->materialGroup.Release(GetEngine());
			delete group;
		}
	}

}

void MRenderableMeshManager::OnTransformChanged(MComponent* pComponent)
{
	if (MRenderableMeshComponent* pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>())
	{
		UpdateMeshInstance(pMeshComponent, MRenderableMaterialGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MRenderableMeshManager::OnMaterialChanged(MComponent* pComponent)
{
	MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>();
	if (!pMeshComponent)
	{
		return;
	}

    RemoveComponentFromGroup(pMeshComponent);

	auto pMaterial = pMeshComponent->GetMaterial();
	if (!pMaterial)
	{
		return;
	}

	if (!IsRenderableMeshMaterial(pMaterial->GetMaterialType()))
	{
		return;
	}

    AddComponentToGroup(pMeshComponent);
}

void MRenderableMeshManager::OnMeshChanged(MComponent* pComponent)
{
	if (MRenderableMeshComponent* pMeshComponent = pComponent->DynamicCast<MRenderableMeshComponent>())
	{
		//TODO remove it.
		GetEngine()->FindGlobalObject<MMeshManager>()->RegisterMesh(pMeshComponent->GetMesh());

		UpdateMeshInstance(pMeshComponent, MRenderableMaterialGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MRenderableMeshManager::OnVisibleChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderableMeshComponent>())
	{
		UpdateMeshInstance(pMeshComponent, MRenderableMaterialGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MRenderableMeshManager::RemoveComponent(MRenderableMeshComponent* pComponent)
{
	RemoveComponentFromGroup(pComponent);
}

std::vector<MRenderableMaterialGroup*> MRenderableMeshManager::FindGroupFromMaterialType(MEMaterialType eType) const
{
	std::vector<MRenderableMaterialGroup*> vRenderableGroup;
	for (auto pr : m_tRenderableMaterialGroup)
	{
		auto pMaterial = pr.first;
		if (pMaterial->GetMaterialType() == eType)
		{
			vRenderableGroup.push_back(&pr.second->materialGroup);
		}
	}

	return vRenderableGroup;
}

std::vector<MRenderableMaterialGroup*> MRenderableMeshManager::GetAllMaterialGroup() const
{
	std::vector<MRenderableMaterialGroup*> vRenderableGroup;
	for (auto pr : m_tRenderableMaterialGroup)
	{
		vRenderableGroup.push_back(&pr.second->materialGroup);
	}

	return vRenderableGroup;
}

bool MRenderableMeshManager::IsRenderableMeshMaterial(MEMaterialType eType) const
{
	return eType == MEMaterialType::EDefault
		|| eType == MEMaterialType::EDeferred
        || eType == MEMaterialType::ECustom;
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
		findResult->second->tWaitAddComponent[pComponent] = MRenderableMaterialGroup::CreateProxyFromComponent(pComponent);
		m_tComponentTable[pComponent] = findResult->second;
		return;
	}

	auto pRenderableGroup = new MaterialGroup();
	m_tRenderableMaterialGroup[pMaterial] = pRenderableGroup;
	pRenderableGroup->materialGroup.Initialize(GetEngine(), pMaterial);

	pRenderableGroup->tWaitAddComponent[pComponent] = MRenderableMaterialGroup::CreateProxyFromComponent(pComponent);
	m_tComponentTable[pComponent] = pRenderableGroup;
}

void MRenderableMeshManager::RemoveComponentFromGroup(MRenderableMeshComponent* pComponent)
{
	const auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	auto group = findResult->second;

	group->tWaitAddComponent.erase(pComponent);
	group->tWaitUpdateComponent.erase(pComponent);
	group->tWaitRemoveComponent.insert(pComponent);
	m_tComponentTable.erase(findResult);
}

void MRenderableMeshManager::UpdateMeshInstance(MRenderableMeshComponent* pComponent, MMeshInstanceRenderProxy proxy)
{
	const auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	auto pMaterialGroup = findResult->second;
	if (!pMaterialGroup)
	{
		MORTY_ASSERT(pMaterialGroup);
		return;
	}

	pMaterialGroup->tWaitUpdateComponent[pComponent] = proxy;
}

void MRenderableMeshManager::Clean()
{
	for (auto pr : m_tRenderableMaterialGroup)
	{
		auto* group = pr.second;
		group->materialGroup.Release(GetEngine());
		delete group;
	}

	m_tRenderableMaterialGroup.clear();
	m_tComponentTable.clear();
}
