#include "MEntity.h"
#include "MScene.h"
#include "MEngine.h"
#include "MVariant.h"
#include "MFileHelper.h"
#include "MObjectSystem.h"

#include "MScene.h"
#include "MComponent.h"

#include "MFunction.h"

MORTY_CLASS_IMPLEMENT(MEntity, MTypeClass)

MEntity::MEntity()
	: MTypeClass()
	, m_pScene(nullptr)
	, m_id(0)
	, m_strName("")
{
}

MEntity::MEntity(MScene* pScene, const MEntityID& nID)
	: MTypeClass()
	, m_pScene(pScene)
	, m_id(nID)
	, m_strName("")
{

}

MEntity::~MEntity()
{

}

void MEntity::UnregisterComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return;

	m_pScene->RemoveComponent(this, pComponentType);
}

void MEntity::UnregisterAllComponent()
{
	std::vector<MComponentID> vect = m_vComponents;

	for (const MComponentID& id : vect)
	{
		UnregisterComponent(id.pComponentType);
	}
}

bool MEntity::HasComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return false;

	for (const MComponentID& id : m_vComponents)
	{
		if (id.pComponentType == pComponentType)
			return true;
	}

	return false;
}

MComponent* MEntity::GetComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return nullptr;

	return m_pScene->FindComponent(this, pComponentType);
}

std::vector<MComponent*> MEntity::GetComponents()
{
	std::vector<MComponent*> vResult;

	for (auto id : m_vComponents)
	{
		vResult.push_back(m_pScene->GetComponent(id));
	}

	return vResult;
}

MEngine* MEntity::GetEngine()
{
	if (m_pScene)
	{
		return m_pScene->GetEngine();
	}

	return nullptr;
}

void MEntity::DeleteSelf()
{
	if (m_pScene)
	{
		m_pScene->DeleteEntity(this);
	}
}

void MEntity::WriteToStruct(MStruct& srt)
{
	MSerializer::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Name", GetName);

	if (MVariantArray* pComponentArray = FindWriteVariant<MVariantArray>(*pStruct, "Components"))
	{
		std::vector<MComponent*>& vComponents = GetComponents();
		for (MComponent* pComponent : vComponents)
		{
			if (MStruct* pCompStruct = pComponentArray->AppendMVariant<MStruct>())
			{
				pCompStruct->AppendMVariant("ComponentType", pComponent->GetTypeName());
				pComponent->WriteToStruct(*pCompStruct);
			}
		}
	}

	M_SERIALIZER_END;
}

void MEntity::ReadFromStruct(const MStruct& srt)
{
	MSerializer::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Name", SetName, String);

	if (const MVariantArray* pComponentArray = FindReadVariant<MVariantArray>(*pStruct, "Components"))
	{
		for (size_t i = 0; i < pComponentArray->GetMemberCount(); ++i)
		{
			if (const MStruct* pCompStruct = pComponentArray->GetMember<MStruct>(i))
			{
				if (const MString* pComponentTypeName = pCompStruct->FindMember<MString>("ComponentType"))
				{
					if (MComponent* pComponent = GetScene()->AddComponent(this, MTypeClass::GetType(*pComponentTypeName)))
					{
						pComponent->ReadFromStruct(*pCompStruct);
					}
				}
			}
		}
	}

	M_SERIALIZER_END;
}
