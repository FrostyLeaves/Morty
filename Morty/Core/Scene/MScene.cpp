#include "MScene.h"
#include "MFunction.h"
#include "MEngine.h"
#include "MComponent.h"

#include "MComponentSystem.h"

MORTY_CLASS_IMPLEMENT(MScene, MObject)

static auto ComponentIDLessFunction = [](const MComponentID& a, const MComponentID& b) {
	return a.pComponentType < b.pComponentType;
};


MScene::MScene()
	: MObject()
	, m_tComponents()
{
	
}

MScene::~MScene()
{

}

MEntity* MScene::CreateEntity()
{
	MEntity* pEntity = new MEntity(this, m_vEntity.size());
	m_vEntity.push_back(pEntity);

	return pEntity;
}

void MScene::DeleteEntity(MEntity* pEntity)
{
	//Delete Components.
	if (!pEntity)
		return;

	pEntity->UnregisterAllComponent();

	m_vEntity.erase(std::remove(m_vEntity.begin(), m_vEntity.end(), pEntity));
}

MEntity* MScene::GetEntity(const MEntityID& id)
{
	if (id < m_vEntity.size())
		return m_vEntity[id];
}

void MScene::OnCreated()
{
	MObject::OnCreated();

}

void MScene::OnDelete()
{
	//TODO remove all component.

	Super::OnDelete();
}

MEntity* MScene::FindFirstEntityByComponent(const MType* pComponentType)
{
	MIComponentGroup* pGroup = FindComponents(pComponentType);

	if (!pGroup)
		return nullptr;

	MComponent* pComponent = pGroup->FirstComponent();
	if (!pComponent)
		return nullptr;

	return pComponent->GetEntity();
}

MIComponentGroup* MScene::FindComponents(const MType* pComponentType)
{
	auto findResult = m_tComponents.find(pComponentType);
	if (findResult == m_tComponents.end())
	{
		if (MIComponentGroup* pGroup = CreateComponents(pComponentType))
		{
			return m_tComponents[pComponentType] = pGroup;
		}

		return nullptr;
	}

	return findResult->second;
}

MComponent* MScene::FindComponent(MEntity* entity, const MType* pComponentType)
{
	if (MIComponentGroup* pComponents = FindComponents(pComponentType))
	{
		auto iter = std::find(entity->m_vComponents.begin(), entity->m_vComponents.end(), pComponentType);
		if (iter == entity->m_vComponents.end())
			return nullptr;

		return pComponents->FindComponent(iter->nID);
	}

	return nullptr;
}

MComponent* MScene::GetComponent(const MComponentID& id)
{
	if (!id.IsValid())
		return nullptr;

	if (MIComponentGroup* pComponents = FindComponents(id.pComponentType))
	{
		return pComponents->FindComponent(id.nID);
	}

	return nullptr;
}

void MScene::Tick(const float& fDelta)
{
	MEngine* pEngine = GetEngine();
	if (!pEngine)
		return;

	auto& vSystem = pEngine->GetAllSystem();

	for (MISystem* pSystem : vSystem)
	{
		pSystem->SceneTick(this, fDelta);
	}
}

MComponent* MScene::AddComponent(MEntity* entity, MIComponentGroup* pComponents)
{
	if (!entity)
		return nullptr;

	MComponentID compID = pComponents->AddComponent(entity);
	UNION_ORDER_PUSH_BACK_VECTOR<MComponentID>(entity->m_vComponents, compID, ComponentIDLessFunction);
	MComponent* pComponent = pComponents->FindComponent(compID.nID);
	return pComponent;
}

MComponent* MScene::AddComponent(MEntity* entity, const MType* pComponentType)
{
	if (MIComponentGroup* pComponents = FindComponents(pComponentType))
	{
		return AddComponent(entity, pComponents);
	}

	return nullptr;
}

MIComponentGroup* MScene::CreateComponents(const MType* pComponentType)
{
	if (MComponentSystem* pComponentSystem = GetEngine()->FindSystem<MComponentSystem>())
	{
		MIComponentGroup* pGroup = pComponentSystem->CreateComponentGroup(pComponentType);
		if (pGroup)
		{
			pGroup->m_pScene = this;
		}
		return pGroup;
	}

	return nullptr;
}

void MScene::RemoveComponent(MEntity* entity, const MType* pComponentType)
{
	if (!entity)
		return;

	MIComponentGroup* pComponents = FindComponents(pComponentType);
	if (!pComponents)
		return;

	MComponentID id(pComponentType, 0);
	size_t nIdx = FIND_ORDER_VECTOR<MComponentID, MComponentID>(entity->m_vComponents, id, ComponentIDLessFunction);

	id = entity->m_vComponents[nIdx];

	if (id.pComponentType == pComponentType)
	{
		entity->m_vComponents.erase(entity->m_vComponents.begin() + nIdx);
		pComponents->RemoveComponent(id.nID);
	}
}
