#include "MComponent.h"

MORTY_CLASS_IMPLEMENT(MComponent, MTypeClass)

#include "MScene.h"
#include "MEntity.h"

#include "MNotifySystem.h"

MComponent::MComponent()
	: MTypeClass()
	, m_id()
	, m_entityID()
	, m_pScene(nullptr)
	, m_bValid(false)
{

}

MComponent::~MComponent()
{

}

void MComponent::Initialize(MScene* pScene, const MEntityID& id)
{
	m_pScene = pScene;
	m_entityID = id;

	Initialize();
}

void MComponent::Initialize()
{
	m_bValid = true;
}

void MComponent::Release()
{
	m_bValid = false;
}

void MComponent::WriteToStruct(MStruct& srt, MComponentRefTable& refTable)
{
	M_SERIALIZER_WRITE_BEGIN

	uint32_t nIndex = refTable.FindOrAdd(GetComponentID());
	srt.AppendMVariant("RefIndex", int(nIndex));

	M_SERIALIZER_END
}

void MComponent::ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable)
{
	M_SERIALIZER_READ_BEGIN

	uint32_t nIndex = *srt.FindMember<int>("RefIndex");
	refTable.AddRef(GetComponentID(), nIndex);

	M_SERIALIZER_END
}

void MComponent::SendComponentNotify(const MString& notify)
{
	if (MEngine* pEngine = GetEngine())
	{
		if (MNotifySystem* pNotifySystem = pEngine->FindSystem<MNotifySystem>())
		{
			pNotifySystem->SendNotify(notify, GetScene(), GetComponentID());
		}
	}
}

MEntity* MComponent::GetEntity() const
{
	if (m_pScene)
	{
		return m_pScene->GetEntity(m_entityID);
	}
	return nullptr;
}

MEngine* MComponent::GetEngine() const
{
	if (!m_pScene)
		return nullptr;

	return m_pScene->GetEngine();
}

bool MComponentID::operator==(const MComponentID& id) const
{
	return pComponentType == id.pComponentType && nPrimaryIdx == id.nPrimaryIdx && nSecondaryIdx == id.nSecondaryIdx;
}

bool MComponentID::operator==(const MType* pType) const
{
	return pComponentType == pType;
}

bool MComponentID::operator<(const MComponentID& id) const
{
	if (pComponentType < id.pComponentType)
		return true;
	else if (pComponentType > id.pComponentType)
		return false;

	if (nPrimaryIdx < id.nPrimaryIdx)
		return true;
	else if (nPrimaryIdx > id.nPrimaryIdx)
		return false;

	return nSecondaryIdx < id.nSecondaryIdx;
}

bool MComponentID::operator<(const MType* pType) const
{
	return pComponentType < pType;
}

bool MComponentID::IsValid() const
{
	return pComponentType;
}

MComponentRefTable::MComponentRefTable(MScene* pScene)
	: m_pScene(pScene)
{
}

void MComponentRefTable::AddRef(const MComponentID& compID, const uint32_t& nIndex)
{
	m_tIndexToRef[nIndex] = compID;
	m_tRefToIndex[compID] = nIndex;
}

MComponentID MComponentRefTable::Find(const uint32_t& nIndex)
{
	auto findResult = m_tIndexToRef.find(nIndex);

	if (findResult == m_tIndexToRef.end())
		return MComponentID();

	return findResult->second;
}

uint32_t MComponentRefTable::FindOrAdd(const MComponentID& compID)
{
	if (!compID.IsValid())
		return 0;

	auto findResult = m_tRefToIndex.find(compID);

	if (findResult == m_tRefToIndex.end())
	{
		m_tRefToIndex[compID] = ++m_Count;
		m_tIndexToRef[m_Count] = compID;
		return m_Count;
	}

	return findResult->second;
}

void MComponentRefTable::BindReference()
{
	for (auto& f : m_vRefBindFunction)
	{
		MComponent* pComponent = m_pScene->GetComponent(Find(f.nIndex));
		f.func(pComponent);
	}
}

void MComponentRefTable::AddReferenceFunction(std::function<void(MComponent* pSerializer)> func, const uint32_t& nIdx)
{
	RefBindFunction f;
	f.func = func;
	f.nIndex = nIdx;
	m_vRefBindFunction.push_back(f);
}
