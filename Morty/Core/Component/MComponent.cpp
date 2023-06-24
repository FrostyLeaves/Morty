#include "Component/MComponent.h"

MORTY_CLASS_IMPLEMENT(MComponent, MTypeClass)

#include "Scene/MScene.h"
#include "Scene/MEntity.h"

#include "System/MNotifyManager.h"

#include "Flatbuffer/MComponent_generated.h"

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

void MComponent::Initialize(MScene* pScene, const MGuid& id)
{
	m_pScene = pScene;
	m_entityID = id;

	m_bValid = true;
}

void MComponent::Release()
{
	m_bValid = false;
}

flatbuffers::Offset<void> MComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	mfbs::MComponentBuilder compBuilder(fbb);

	return compBuilder.Finish().Union();
}

void MComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MComponent* fbcomponent = reinterpret_cast<const mfbs::MComponent*>(pBufferPointer);
}

void MComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MComponent* fbcomponent = mfbs::GetMComponent(fbb.GetCurrentBufferPointer());
}

void MComponent::PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid)
{
}

void MComponent::SendComponentNotify(const char* notify)
{
	if (MScene* pScene = GetScene())
	{
		if (MNotifyManager* pNotifySystem = pScene->GetManager<MNotifyManager>())
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
	return pComponentType == id.pComponentType && nIdx == id.nIdx;
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

	if (nIdx < id.nIdx)
		return true;

	return false;
}

bool MComponentID::operator<(const MType* pType) const
{
	return pComponentType < pType;
}

bool MComponentID::IsValid() const
{
	return pComponentType;
}
