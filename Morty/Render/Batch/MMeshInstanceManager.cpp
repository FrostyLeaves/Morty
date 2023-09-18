#include "MMeshInstanceManager.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Utility/MFunction.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"

#include "Engine/MEngine.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MNotifyManager.h"

#include "MRenderNotify.h"
#include "Module/MCoreNotify.h"

#include "Mesh/MMeshManager.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MGlobal.h"

MORTY_INTERFACE_IMPLEMENT(MMeshInstanceManager, IManager)

void MMeshInstanceManager::Initialize()
{
	Super::Initialize();

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_VISIBLE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this));
		pNotifySystem->RegisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_ATTACHED_SKELETON_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnRenderMeshChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMaterialChanged, this));
		pNotifySystem->RegisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMeshChanged, this));
	}

	m_pUpdateTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>("RenderMeshManagerUpdate");
	if (m_pUpdateTask)
	{
		m_pUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::RenderUpdate, this));
		m_pUpdateTask->SetThreadType(METhreadType::ERenderThread);
	}
}

void MMeshInstanceManager::Release()
{
	if (m_pUpdateTask)
	{
		GetEngine()->GetMainGraph()->DestroyNode(m_pUpdateTask);
		m_pUpdateTask = nullptr;
	}

	if (MNotifyManager* pNotifySystem = GetScene()->GetManager<MNotifyManager>())
	{
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_VISIBLE_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this));
		pNotifySystem->UnregisterNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnSceneComponentChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_ATTACHED_SKELETON_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnRenderMeshChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMaterialChanged, this));
		pNotifySystem->UnregisterNotify(MRenderNotify::NOTIFY_MESH_CHANGED, M_CLASS_FUNCTION_BIND_0_1(MMeshInstanceManager::OnMeshChanged, this));
	}

	Clean();

	Super::Release();
}

void MMeshInstanceManager::RenderUpdate(MTaskNode* pNode)
{
	MORTY_UNUSED(pNode);

	std::vector<std::shared_ptr<MMaterial>> vDeleteMaterials;
	
	for (auto [material, group] : m_tRenderableMaterialGroup)
	{
		if (!group->tWaitRemoveComponent.empty())
		{
			for (const auto& key : group->tWaitRemoveComponent)
			{
				group->materialGroup.RemoveMeshInstance(key);
			}
			group->tWaitRemoveComponent.clear();
		}

		if (!group->tWaitUpdateComponent.empty())
		{
			for (auto& [key, proxy] : group->tWaitUpdateComponent)
			{
				group->materialGroup.UpdateOrCreateMeshInstance(proxy);
			}
			group->tWaitUpdateComponent.clear();
		}

		if (group->materialGroup.IsEmpty())
		{
			group->materialGroup.Release(GetEngine());

		    MORTY_SAFE_DELETE(group);
			vDeleteMaterials.push_back(material);
		}
	}

	for (auto& material : vDeleteMaterials)
	{
		m_tRenderableMaterialGroup.erase(material);
	}

}

void MMeshInstanceManager::OnMaterialChanged(MComponent* pComponent)
{
	MRenderMeshComponent* pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>();
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

void MMeshInstanceManager::OnMeshChanged(MComponent* pComponent)
{
	if (MRenderMeshComponent* pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
	{
		//TODO remove it.
		GetEngine()->FindGlobalObject<MMeshManager>()->RegisterMesh(pMeshComponent->GetMesh());

		UpdateMeshInstance(pMeshComponent, MMaterialBatchGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MMeshInstanceManager::OnRenderMeshChanged(MComponent* pComponent)
{
	if (auto pMeshComponent = pComponent->template DynamicCast<MRenderMeshComponent>())
	{
		UpdateMeshInstance(pMeshComponent, MMaterialBatchGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MMeshInstanceManager::OnSceneComponentChanged(MComponent* pComponent)
{
	if (MRenderMeshComponent* pMeshComponent = pComponent->GetEntity()->GetComponent<MRenderMeshComponent>())
	{
		UpdateMeshInstance(pMeshComponent, MMaterialBatchGroup::CreateProxyFromComponent(pMeshComponent));
	}
}

void MMeshInstanceManager::RemoveComponent(MRenderMeshComponent* pComponent)
{
	RemoveComponentFromGroup(pComponent);
}

std::vector<MMaterialBatchGroup*> MMeshInstanceManager::FindGroupFromMaterialType(MEMaterialType eType) const
{
	std::vector<MMaterialBatchGroup*> vRenderableGroup;
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

std::vector<MMaterialBatchGroup*> MMeshInstanceManager::GetAllMaterialGroup() const
{
	std::vector<MMaterialBatchGroup*> vRenderableGroup;
	for (auto pr : m_tRenderableMaterialGroup)
	{
		vRenderableGroup.push_back(&pr.second->materialGroup);
	}

	return vRenderableGroup;
}

bool MMeshInstanceManager::IsRenderableMeshMaterial(MEMaterialType eType) const
{
	return eType == MEMaterialType::EDefault
		|| eType == MEMaterialType::EDeferred
        || eType == MEMaterialType::ECustom;
}

void MMeshInstanceManager::AddComponentToGroup(MRenderMeshComponent* pComponent)
{
	const auto pMaterial = pComponent->GetMaterial();
	if (nullptr == pMaterial)
	{
		return;
	}

	const auto findResult = m_tRenderableMaterialGroup.find(pMaterial);
	if (findResult != m_tRenderableMaterialGroup.end())
	{
		auto proxy = MMaterialBatchGroup::CreateProxyFromComponent(pComponent);
		findResult->second->tWaitUpdateComponent[proxy.nProxyId] = proxy;
		m_tComponentTable[pComponent] = findResult->second;
		return;
	}

	auto pRenderableGroup = new MaterialGroup();
	m_tRenderableMaterialGroup[pMaterial] = pRenderableGroup;
	pRenderableGroup->materialGroup.Initialize(GetEngine(), pMaterial);

	auto proxy = MMaterialBatchGroup::CreateProxyFromComponent(pComponent);
	pRenderableGroup->tWaitUpdateComponent[proxy.nProxyId] = proxy;
	m_tComponentTable[pComponent] = pRenderableGroup;
}

void MMeshInstanceManager::RemoveComponentFromGroup(MRenderMeshComponent* pComponent)
{
	const auto findResult = m_tComponentTable.find(pComponent);
	if (findResult == m_tComponentTable.end())
	{
		return;
	}

	auto group = findResult->second;

	auto nProxyId = pComponent->GetComponentID().nIdx;
	group->tWaitUpdateComponent.erase(nProxyId);
	group->tWaitRemoveComponent.insert(nProxyId);
	m_tComponentTable.erase(findResult);
}

void MMeshInstanceManager::UpdateMeshInstance(MRenderMeshComponent* pComponent, MMeshInstanceRenderProxy proxy)
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

	pMaterialGroup->tWaitUpdateComponent[proxy.nProxyId] = proxy;
}

void MMeshInstanceManager::Clean()
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
