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

class MORTY_API MScene : public MObject
{
public:
	MORTY_CLASS(MScene);
    MScene();
    virtual ~MScene();

public:

	MEntity* CreateEntity();
	void DeleteEntity(MEntity* pEntity);

	MEntity* GetEntity(const MEntityID& id);

	template<typename TYPE>
	MEntity* FindFirstEntityByComponent();
	MEntity* FindFirstEntityByComponent(const MType* pComponentType);

public:

	template <class TYPE>
	MComponentGroup<TYPE>* FindComponents();
	MIComponentGroup* FindComponents(const MType* pComponentType);

	MComponent* FindComponent(MEntity* entity, const MType* pComponentType);

	MComponent* GetComponent(const MComponentID& id);

	std::vector<MEntity*> GetAllEntity() const;

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

	MIComponentGroup* CreateComponents(const MType* pComponentType);

private:

	std::map<MEntityID, MEntity*> m_vEntity;
	std::map<const MType*, MIComponentGroup*> m_tComponents;
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
