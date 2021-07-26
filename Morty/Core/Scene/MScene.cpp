#include "MScene.h"
#include "MFunction.h"
#include "MEngine.h"
#include "MComponent.h"

MORTY_CLASS_IMPLEMENT(MScene, MObject)

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

MIComponentGroup* MScene::FindComponents(const MType* pComponentType)
{
	auto findResult = m_tComponents.find(pComponentType);
	if (findResult == m_tComponents.end())
		return nullptr;

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
	if (MIComponentGroup* pComponents = FindComponents(id.pComponentType))
	{
		return pComponents->FindComponent(id.nID);
	}

	return nullptr;
}

void MScene::Tick(const float& fDelta)
{
}

MComponent* MScene::AddComponent(MEntity* entity, MIComponentGroup* pComponents)
{
	if (!entity)
		return nullptr;

	MComponentID compID = pComponents->AddComponent(entity);
	UNION_ORDER_PUSH_BACK_VECTOR<MComponentID>(entity->m_vComponents, compID);
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

void MScene::RemoveComponent(MEntity* entity, const MType* pComponentType)
{
	if (!entity)
		return;

	MIComponentGroup* pComponents = FindComponents(pComponentType);
	if (!pComponents)
		return;

	MComponentID id(pComponentType, 0);
	ERASE_UNION_ORDER_VECTOR<MComponentID>(entity->m_vComponents, id
		, [](const MComponentID& a, const MComponentID& b) {
		return a.pComponentType < b.pComponentType;
		}, [](const MComponentID& a, const MComponentID& b) {
			return a.pComponentType == b.pComponentType;
		});

	pComponents->RemoveComponent(entity->GetID());
}
