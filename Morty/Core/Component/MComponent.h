/**
 * @File         MComponent
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOMPONENT_H_
#define _M_MCOMPONENT_H_
#include "Utility/MGlobal.h"
#include "Type/MType.h"

#include "Scene/MGuid.h"
#include "Utility/MSerializer.h"
#include "Utility/MBlockVector.h"

#include "flatbuffers/flatbuffer_builder.h"

class MScene;
class MEntity;
class MEngine;
class MComponent;

class MORTY_API MComponentID
{
public:
    MComponentID() : pComponentType(nullptr), nPrimaryIdx(0), nSecondaryIdx(0) {}
    MComponentID(const MType* type, const size_t& pid, const size_t& sid) : pComponentType(type), nPrimaryIdx(pid), nSecondaryIdx(sid) {}

    bool IsValid() const;

    bool operator ==(const MComponentID& id) const;
    bool operator ==(const MType* pType) const;

	bool operator < (const MComponentID& id) const;
	bool operator < (const MType* pType) const;

public:
    const MType* pComponentType;
	size_t nPrimaryIdx;
    size_t nSecondaryIdx;
};

class MORTY_API MComponent : public MTypeClass
{
public:
    MORTY_CLASS(MComponent)

public:
    MComponent();
    virtual ~MComponent();

public:

    void Initialize(MScene* pScene, const MGuid& id);
    virtual void Release();

    bool IsValid() const { return m_bValid; }


public:

    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb);
    virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb);
	virtual void Deserialize(const void* pBufferPointer);
	virtual void PostDeserialize();

public:

    void SendComponentNotify(const MString& notify);

	MScene* GetScene() const { return m_pScene; }
	MEntity* GetEntity() const;
    MEngine* GetEngine() const;

    void SetComponentID(const MComponentID& id) { m_id = id; }
    const MComponentID& GetComponentID() { return m_id; }

private:
//property
	MComponentID m_id;

private:
	MGuid m_entityID;
    MScene* m_pScene;

    bool m_bValid;
};

class MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() = 0;
	virtual MComponentID AddComponent(MEntity* entity) = 0;
	virtual void RemoveComponent(const MComponentID& id) = 0;
	virtual void RemoveAllComponents() = 0;

	virtual MComponent* FindComponent(const size_t& pid, const size_t& sid) = 0;

	MComponent* FindComponent(const MComponentID& id) {
		return FindComponent(id.nPrimaryIdx, id.nSecondaryIdx);
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
	virtual MComponent* FindComponent(const size_t& pid, const size_t& sid) override;;

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

		MComponentID id = MComponentID(TYPE::GetClassType(), iter.pid(), iter.sid());

		component.SetComponentID(id);
		component.MComponent::Initialize(m_pScene, entity->GetID());
		return id;
	}

	MComponentID nResult = m_vFreeComponent.back();
	m_vFreeComponent.pop_back();
	
	MComponent* pComponent = FindComponent(nResult.nPrimaryIdx, nResult.nSecondaryIdx);
	MORTY_ASSERT(pComponent && !pComponent->IsValid());

	pComponent->SetComponentID(nResult);
	pComponent->MComponent::Initialize(m_pScene, entity->GetID());
	return nResult;
}

template<typename TYPE>
void MComponentGroup<TYPE>::RemoveComponent(const MComponentID& id)
{
	MComponent* pComponent = FindComponent(id.nPrimaryIdx, id.nSecondaryIdx);
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
MComponent* MComponentGroup<TYPE>::FindComponent(const size_t& pid, const size_t& sid)
{
	return m_vComponents.get(pid, sid);
}

#endif
