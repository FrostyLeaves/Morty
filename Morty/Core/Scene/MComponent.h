/**
 * @File         MComponent
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOMPONENT_H_
#define _M_MCOMPONENT_H_
#include "MGlobal.h"
#include "MType.h"

#include "MSerializer.h"

class MScene;
class MEntity;
class MEngine;
class MComponent;

class MORTY_API MComponentID
{
public:
    MComponentID() : pComponentType(nullptr), nID(0) {}
    MComponentID(const MType* type, const size_t& id) : pComponentType(type), nID(id) {}

    bool IsValid() const;

    bool operator ==(const MComponentID& id) const;
    bool operator ==(const MType* pType) const;

	bool operator < (const MComponentID& id) const;
	bool operator < (const MType* pType) const;

public:
    const MType* pComponentType;
    size_t nID;
};

class MORTY_API MComponentRefTable
{
public:

	MComponentRefTable(MScene* pScene);

	struct RefBindFunction {
		std::function<void(MComponent* pSerializer)> func;
		uint32_t nIndex;
	};

	void AddRef(const MComponentID& compID, const uint32_t& nIndex);
    MComponentID Find(const uint32_t& nIndex);
	uint32_t FindOrAdd(const MComponentID& compID);

	void BindReference();

	void AddReferenceFunction(std::function<void(MComponent* pSerializer)> func, const uint32_t& nIdx);

public:
    MScene* m_pScene;

	std::map<MComponentID, uint32_t> m_tRefToIndex;
	std::map<uint32_t, MComponentID> m_tIndexToRef;

	uint32_t m_Count = 0;

	std::vector<RefBindFunction> m_vRefBindFunction;
};

#define M_SERIALIZER_WRITE_COMPONENT_REF( NAME, REF_GET_FUNC) \
	MComponent* pComponent = REF_GET_FUNC(); \
	pStruct->AppendMVariant(NAME, int(refTable.FindOrAdd(pComponent ? pComponent->GetComponentID() : MComponentID()))); \

#define M_SERIALIZER_READ_COMPONENT_REF( NAME, REF_SET_FUNC, TYPE) \
	if(const MVariant* pVariant = pStruct->FindMember(NAME)) \
		if(auto pValue = pVariant->GetInt()) {\
			uint32_t nIndex = *pValue; \
			MScene* pScene = GetScene(); \
			MComponentID self = GetComponentID(); \
			refTable.AddReferenceFunction([=](MComponent* target){ \
				if(!target) return; \
				if (MComponent* pSelfBase = pScene->GetComponent(self)){ \
					if(Class* pSelfComponent = pSelfBase->DynamicCast<Class>()) { \
						pSelfComponent->REF_SET_FUNC(target->DynamicCast<TYPE>()); \
					} \
				} \
			}, nIndex);\
		}

class MORTY_API MComponent : public MTypeClass
{
public:
    MORTY_CLASS(MComponent)

public:
    MComponent();
    virtual ~MComponent();

public:

    void Initialize(MScene* pScene, const MEntityID& id);

    bool IsValid() const { return m_bValid; }

    virtual void Initialize();
    virtual void Release();

public:

    virtual void WriteToStruct(MStruct& srt, MComponentRefTable& refTable);

    virtual void ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable);

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
	MEntityID m_entityID;
    MScene* m_pScene;

    bool m_bValid;
};

class MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() = 0;
	virtual MComponentID AddComponent(MEntity* entity) = 0;
	virtual void RemoveComponent(const size_t& id) = 0;

	virtual MComponent* FindComponent(const size_t& id) = 0;

public:
	MScene* m_pScene = nullptr;
};

template<typename TYPE>
class MORTY_API MComponentGroup : public MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() override;
	virtual MComponentID AddComponent(MEntity* entity) override;
	virtual void RemoveComponent(const size_t& id) override;
	virtual MComponent* FindComponent(const size_t& id) override;

	std::vector<size_t> m_vFreeComponent;
	std::vector<TYPE> m_vComponent;
};

template<typename TYPE>
MComponent* MComponentGroup<TYPE>::FirstComponent()
{
	for (TYPE& comp : m_vComponent)
	{
		if (comp.IsValid())
			return &comp;
	}

	return nullptr;
}

template<typename TYPE>
MComponentID MComponentGroup<TYPE>::AddComponent(MEntity* entity)
{
	if (m_vFreeComponent.empty())
	{
		size_t nResult = m_vComponent.size();
		m_vComponent.resize(nResult + 1);
		MComponentID id = MComponentID(TYPE::GetClassType(), nResult);
		m_vComponent[nResult].SetComponentID(id);
		m_vComponent[nResult].MComponent::Initialize(m_pScene, entity->GetID());
		return id;
	}

	size_t nResult = m_vFreeComponent.back();
	m_vFreeComponent.pop_back();
	MComponentID id = MComponentID(TYPE::GetClassType(), nResult);
	m_vComponent[nResult].SetComponentID(id);
	m_vComponent[nResult].MComponent::Initialize(m_pScene, entity->GetID());
	return id;
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

	return nullptr;
}

#endif
