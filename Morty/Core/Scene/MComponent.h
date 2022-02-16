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
	virtual void RemoveComponent(const MComponentID& id) = 0;

	virtual MComponent* FindComponent(const size_t& pid, const size_t& sid) = 0;

	MComponent* FindComponent(const MComponentID& id) {
		return FindComponent(id.nPrimaryIdx, id.nSecondaryIdx);
	}

public:
	MScene* m_pScene = nullptr;
};

template<typename TYPE>
struct MORTY_API MComponentVector
{
public:

	static const size_t ArraySize = 64;

	std::array<TYPE, MComponentVector::ArraySize> vComponents = {};
	size_t nMaxAllowIndex = 0;
};

template<typename TYPE>
class MORTY_API MComponentGroup : public MIComponentGroup
{
public:
	virtual MComponent* FirstComponent() override;
	virtual MComponentID AddComponent(MEntity* entity) override;
	virtual void RemoveComponent(const MComponentID& id) override;
	virtual MComponent* FindComponent(const size_t& pid, const size_t& sid) override;


private:
	std::vector<MComponentID> m_vFreeComponent;
	std::vector<MComponentVector<TYPE>> m_vComponentVector;
};

template<typename TYPE>
MComponent* MComponentGroup<TYPE>::FirstComponent()
{
	for (auto iter = m_vComponentVector.begin(); iter != m_vComponentVector.end(); ++iter)
	{
		for (auto it = iter->vComponents.begin(); it != iter->vComponents.end(); ++it)
		{
			if (it->IsValid())
			{
				return &(*it);
			}
		}
	}

	return nullptr;
}

template<typename TYPE>
MComponentID MComponentGroup<TYPE>::AddComponent(MEntity* entity)
{
	if (m_vFreeComponent.empty())
	{
		int nPrimIdx = 0;
		for (auto iter = m_vComponentVector.begin(); iter != m_vComponentVector.end(); ++iter, ++nPrimIdx)
		{
			MComponentVector<TYPE>& compVec = (*iter);
			if (compVec.nMaxAllowIndex < compVec.ArraySize)
			{
				size_t nSecIdx = compVec.nMaxAllowIndex++;

				MComponentID id = MComponentID(TYPE::GetClassType(), nPrimIdx, nSecIdx);
				compVec.vComponents[nSecIdx].SetComponentID(id);
				compVec.vComponents[nSecIdx].MComponent::Initialize(m_pScene, entity->GetID());

				return id;
			}
		}

		m_vComponentVector.push_back(MComponentVector<TYPE>());
		MComponentVector<TYPE>& compVec = m_vComponentVector.back();
		size_t nSecIdx = compVec.nMaxAllowIndex++;

		MComponentID id = MComponentID(TYPE::GetClassType(), nPrimIdx, nSecIdx);
		compVec.vComponents[nSecIdx].SetComponentID(id);
		compVec.vComponents[nSecIdx].MComponent::Initialize(m_pScene, entity->GetID());
		return id;
	}

	MComponentID nResult = m_vFreeComponent.back();
	m_vFreeComponent.pop_back();
	
	MComponent* pComponent = FindComponent(nResult.nPrimaryIdx, nResult.nSecondaryIdx);
	assert(pComponent && !pComponent->IsValid());

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
MComponent* MComponentGroup<TYPE>::FindComponent(const size_t& pid, const size_t& sid)
{
	if (pid < m_vComponentVector.size())
	{
		MComponentVector<TYPE>& compVec = m_vComponentVector[pid];
		if (sid < compVec.ArraySize)
		{
			return &compVec.vComponents[sid];
		}
	}

	return nullptr;
}

#endif
