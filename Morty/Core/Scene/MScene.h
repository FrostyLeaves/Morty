/**
 * @File         MScene
 * 
 * @Created      2019-09-19 00:32:56
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Math/Matrix.h"
#include "Scene/MEntity.h"
#include "Scene/MManager.h"
#include "Engine/MEngine.h"
#include "Component/MComponent.h"
#include "Component/MComponentGroup.h"

MORTY_SPACE_BEGIN

class MTaskNode;
class MResource;
class MORTY_API MScene : public MObject
{
public:
	MORTY_CLASS(MScene);
    MScene();
    virtual ~MScene();

public:

	MEntity* CreateEntity();
	MEntity* CreateEntity(const MGuid& guid);
	void DeleteEntity(MEntity* pEntity);

	MEntity* GetEntity(const MGuid& id);

	template<typename TYPE>
	MEntity* FindFirstEntityByComponent();
	MEntity* FindFirstEntityByComponent(const MType* pComponentType);

public:

	template<typename TYPE>
	TYPE* AddComponent(MEntity* entity);
	MComponent* AddComponent(MEntity* entity, const MType* pComponentType);
	void RemoveComponent(MEntity* entity, const MType* pComponentType);

	template <class TYPE>
	MComponentGroup<TYPE>* FindComponents();
	MIComponentGroup* FindComponents(const MType* pComponentType);

	MComponent* FindComponent(MEntity* entity, const MType* pComponentType);

	MComponent* GetComponent(const MComponentID& id);

	std::vector<MEntity*> GetAllEntity() const;

public:

	template <class TYPE>
	TYPE* RegisterManager();

	template <class TYPE>
	TYPE* GetManager() const;

protected:

	void RegisterManager(const MType* pManagerType, IManager* pManager);

public:

	void Tick(const float& fDelta);

	void OnCreated() override;
	void OnDelete() override;

protected:

	MComponent* AddComponent(MEntity* entity, MIComponentGroup* pComponents);

	MIComponentGroup* CreateComponents(const MType* pComponentType);

private:

	std::map<MGuid, MEntity*> m_vEntity;
	std::map<const MType*, MIComponentGroup*> m_tComponents;
	std::map<const MType*, IManager*> m_tManager;
	std::map<const MType*, std::vector<IManager*>> m_tComponentRegister;
};

template<typename TYPE>
MEntity* MScene::FindFirstEntityByComponent()
{
	return FindFirstEntityByComponent(TYPE::GetClassType());
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

template<class TYPE>
inline TYPE* MScene::RegisterManager()
{
	auto findResult = m_tManager.find(TYPE::GetClassType());
	if (findResult != m_tManager.end())
	{
		return static_cast<TYPE*>(findResult->second);
	}

	TYPE* pManager = new TYPE();
	RegisterManager(TYPE::GetClassType(), pManager);
	pManager->Initialize();
	return pManager;
}

template<class TYPE>
inline TYPE* MScene::GetManager() const
{
	auto findResult = m_tManager.find(TYPE::GetClassType());
	if (findResult != m_tManager.end())
	{
		return static_cast<TYPE*>(findResult->second);
	}

	return nullptr;
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

MORTY_SPACE_END