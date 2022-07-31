/**
 * @File         MEntity
 * 
 * @Created      2021-07-05 11:00:00
 *
 * @Author       DoubleYe
**/

#ifndef _M_MNODE_H_
#define _M_MNODE_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Scene/MGuid.h"
#include "Component/MComponent.h"
#include "Utility/MSerializer.h"
#include "crossguid/guid.hpp"

#include <vector>
#include <functional>

class MScene;
class MORTY_API MEntity : public MTypeClass
{
	MORTY_CLASS(MEntity)
public:
    MEntity();
	MEntity(MScene* pScene, const MGuid& nID);
    virtual ~MEntity();	//Release memory

public:

	MGuid GetID() const { return m_id; }

public:

	void SetName(const MString& strName) { m_strName = strName; }
	MString GetName() const { return m_strName; }

	template <class T>
	T* RegisterComponent();

	template <class T>
	void UnregisterComponent();
	void UnregisterComponent(const MType* pComponentType);
	void UnregisterAllComponent();

	template <class T>
	bool HasComponent();
	bool HasComponent(const MType* pComponentType);

	template <class T>
	T* GetComponent();
	MComponent* GetComponent(const MType* pComponentType);

	std::vector<MComponent*> GetComponents();

	MEngine* GetEngine();
	MScene* GetScene() { return m_pScene; }

	void DeleteSelf();

public:
	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& builder);
	void Deserialize(const void* pBufferPointer);
	void PostDeserialize();

protected:
	MScene* m_pScene;
	MGuid m_id;
	MString m_strName;

	friend class MScene;
	std::vector<MComponentID> m_vComponents;
};

template <class T>
T* MEntity::RegisterComponent()
{
	if (!m_pScene)
		return nullptr;

	if (!m_pScene->AddComponent<T>(this))
		return nullptr;

	return GetComponent<T>();
}

template <class T>
void MEntity::UnregisterComponent()
{
	UnregisterComponent(T::GetClassType());
}

template <class T>
bool MEntity::HasComponent()
{
	return HasComponent(T::GetClassType());
}

template <class T>
T* MEntity::GetComponent()
{
	return static_cast<T*>(GetComponent(T::GetClassType()));
}

#endif