/**
 * @File         MComponentGroup
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Scene/MEntity.h"
#include "MComponent.h"
#include "Container/MBlockVector.h"


class MScene;
class MEntity;
class MEngine;
class MComponent;

class MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() = 0;
	virtual MComponentID AddComponent(MEntity* entity) = 0;
	virtual void RemoveComponent(const MComponentID& id) = 0;
	virtual void RemoveAllComponents() = 0;

	virtual MComponent* FindComponent(size_t id) = 0;

	MComponent* FindComponent(const MComponentID& id) {
		return FindComponent(id.nIdx);
	}

public:
	MScene* m_pScene = nullptr;
};

template<typename TYPE>
class MORTY_API MComponentGroup : public MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() override;
	virtual MComponentID AddComponent(MEntity* entity) override;
	virtual void RemoveComponent(const MComponentID& id) override;
	virtual void RemoveAllComponents() override;
	virtual MComponent* FindComponent(size_t id) override;

public:
	MBlockVector<TYPE, 64> m_vComponents;

private:
	std::vector<MComponentID> m_vFreeComponent;
};

template<typename TYPE>
MComponent* MComponentGroup<TYPE>::FirstComponent()
{
	for (auto iter = m_vComponents.begin(); iter != m_vComponents.end(); ++iter)
	{
		if (iter->IsValid())
		{
			return &(*iter);
		}
	}

	return nullptr;
}

template<typename TYPE>
MComponentID MComponentGroup<TYPE>::AddComponent(MEntity* entity)
{
	if (m_vFreeComponent.empty())
	{
		TYPE& component = m_vComponents.push_back(TYPE());
		auto iter = m_vComponents.end() - 1;

		MComponentID id = MComponentID(TYPE::GetClassType(), iter.id());

		component.SetComponentID(id);
		component.MComponent::Initialize(m_pScene, entity->GetID());
		return id;
	}

	MComponentID nResult = m_vFreeComponent.back();
	m_vFreeComponent.pop_back();
	
	MComponent* pComponent = FindComponent(nResult.nIdx);
	MORTY_ASSERT(pComponent && !pComponent->IsValid());

	pComponent->SetComponentID(nResult);
	pComponent->MComponent::Initialize(m_pScene, entity->GetID());
	return nResult;
}

template<typename TYPE>
void MComponentGroup<TYPE>::RemoveComponent(const MComponentID& id)
{
	MComponent* pComponent = FindComponent(id.nIdx);
	if (!pComponent)
		return;

	if (!pComponent->IsValid())
		return;

	pComponent->Release();
	m_vFreeComponent.push_back(id);
}

template<typename TYPE>
void MComponentGroup<TYPE>::RemoveAllComponents()
{
	for (TYPE& component : m_vComponents)
	{
		if (component.IsValid())
		{
			component.Release();
		}
	}

	m_vComponents.clear();
}

template<typename TYPE>
MComponent* MComponentGroup<TYPE>::FindComponent(size_t id)
{
	return m_vComponents.get(id);
}
