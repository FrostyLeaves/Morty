#include "MComponent.h"

MORTY_CLASS_IMPLEMENT(MComponent, MTypeClass)

#include "MScene.h"
#include "MEntity.h"

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

MEntity* MComponent::GetEntity()
{
	if (m_pScene)
	{
		return m_pScene->GetEntity(m_entityID);
	}
	return nullptr;
}

MEngine* MComponent::GetEngine()
{
	if (!m_pScene)
		return nullptr;

	return m_pScene->GetEngine();
}

bool MComponentID::operator==(const MComponentID& id) const
{
	return pComponentType == id.pComponentType && nID == id.nID;
}

bool MComponentID::operator==(const MType* pType) const
{
	return pComponentType == pType;
}

bool MComponentID::operator<(const MComponentID& id) const
{
	return pComponentType < id.pComponentType ? true : (nID < id.nID);
}

bool MComponentID::operator<(const MType* pType) const
{
	return pComponentType < pType;
}

bool MComponentID::IsValid()
{
	return pComponentType;
}
