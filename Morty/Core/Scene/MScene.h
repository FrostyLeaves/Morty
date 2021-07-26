/**
 * @File         MScene
 * 
 * @Created      2019-09-19 00:32:56
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSCENE_H_
#define _M_MSCENE_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MEntity.h"
#include "Matrix.h"
#include "MEngine.h"
#include "MComponent.h"

#include <vector>
#include <functional>

class MResource;

class MIComponentGroup
{
public:
	virtual MComponentID AddComponent(MEntity* entity) = 0;
	virtual void RemoveComponent(const size_t& id) = 0;

	virtual MComponent* FindComponent(const size_t& id) = 0;
};

template<typename TYPE>
class MORTY_API MComponentGroup : public MIComponentGroup
{
public:

	virtual MComponentID AddComponent(MEntity* entity) override;
	virtual void RemoveComponent(const size_t& id) override;
	virtual MComponent* FindComponent(const size_t& id) override;

public:
	MScene* m_pScene;

	std::vector<size_t> m_vFreeComponent;
	std::vector<TYPE> m_vComponent;
};

class MORTY_API MScene : public MObject
{
public:
	MORTY_CLASS(MScene);
    MScene();
    virtual ~MScene();

public:

	MEntity* CreateEntity();
	MEntity* LoadEntity();
	MEntity* LoadEntity(MResource* pResource, const MComponentID& defaultParentSceneComponent);
	void DeleteEntity(MEntity* pEntity);

	MEntity* GetEntity(const MEntityID& id);

public:

	template <class TYPE>
	MComponentGroup<TYPE>* FindComponents();
	MIComponentGroup* FindComponents(const MType* pComponentType);

	MComponent* FindComponent(MEntity* entity, const MType* pComponentType);

	MComponent* GetComponent(const MComponentID& id);

public:

	virtual void Tick(const float& fDelta);

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	template<typename TYPE>
	TYPE* AddComponent(MEntity* entity);
	MComponent* AddComponent(MEntity* entity, const MType* pComponentType);
	void RemoveComponent(MEntity* entity, const MType* pComponentType);


protected:

	MComponent* AddComponent(MEntity* entity, MIComponentGroup* pComponents);

private:

	std::vector<MEntity*> m_vEntity;
	std::map<const MType*, MIComponentGroup*> m_tComponents;
};


template<typename TYPE>
MComponentID MComponentGroup<TYPE>::AddComponent(MEntity* entity)
{
	if (m_vFreeComponent.empty())
	{
		size_t nResult = m_vComponent.size();
		m_vComponent.resize(nResult + 1);
		m_vComponent[nResult].MComponent::Initialize(m_pScene, entity->GetID());
		return MComponentID(TYPE::GetClassType(), nResult);
	}

	size_t nResult = m_vFreeComponent.back();
	m_vFreeComponent.pop_back();
	m_vComponent[nResult].MComponent::Initialize(m_pScene, entity->GetID());
	return MComponentID(TYPE::GetClassType(), nResult);
}

template<typename TYPE>
void MComponentGroup<TYPE>::RemoveComponent(const size_t& id)
{
	m_vComponent[id].Release();
	m_vFreeComponent.push_back(id);
}

template<typename TYPE>
MComponent* MComponentGroup<TYPE>::FindComponent(const size_t& id)
{
	if (id < m_vComponent.size())
		return &m_vComponent[id];
}

template <typename TYPE>
MComponentGroup<TYPE>* MScene::FindComponents()
{
	MIComponentGroup* pResult = FindComponents(TYPE::GetClassType());

	if (!pResult)
	{
		if (MTypeClass::IsType<TYPE, MComponent>())
		{
			MComponentGroup<TYPE>* pGroup = new MComponentGroup<TYPE>();
			pGroup->m_pScene = this;
			pResult = m_tComponents[TYPE::GetClassType()] = pGroup;
		}
	}

	return static_cast<MComponentGroup<TYPE>*>(pResult);
}

template<typename TYPE>
TYPE* MScene::AddComponent(MEntity* entity)
{
	if (MIComponentGroup* pComponents = FindComponents<TYPE>())
	{
		if (MComponent* pResult = AddComponent(entity, pComponents))
		{
			return static_cast<TYPE*>(pResult);
		}
	}

	return nullptr;
}

#endif
