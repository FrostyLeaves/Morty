#include "MEntity.h"
#include "MScene.h"
#include "MEngine.h"
#include "MVariant.h"
#include "MFileHelper.h"
#include "MObjectSystem.h"

#include "MScene.h"
#include "MComponent.h"

#include "MFunction.h"

#include "Flatbuffer/MEntity_generated.h"

MORTY_CLASS_IMPLEMENT(MEntity, MTypeClass)

MEntity::MEntity()
	: MTypeClass()
	, m_pScene(nullptr)
	, m_id()
	, m_strName("")
{
}

MEntity::MEntity(MScene* pScene, const MGuid& nID)
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

flatbuffers::Offset<void> MEntity::Serialize(flatbuffers::FlatBufferBuilder& builder)
{
	MGuid id = GetID();
	mfbs::MGuid fbguid(id.data[0], id.data[1], id.data[2], id.data[3]);
	auto&& fbname = builder.CreateString(GetName());

	std::vector<flatbuffers::Offset<mfbs::AnyComponent >> vFbComponents;
	std::vector<MComponent*>&& vComponents = GetComponents();
	for (MComponent* pComponent : vComponents)
	{
		flatbuffers::FlatBufferBuilder compBuilder;
		flatbuffers::Offset<void>&& componentRoot = pComponent->Serialize(compBuilder);
		compBuilder.Finish(componentRoot);

		flatbuffers::Offset<flatbuffers::Vector<uint8_t>>&& fbdata = builder.CreateVector(compBuilder.GetBufferPointer(), compBuilder.GetSize());
		flatbuffers::Offset<mfbs::AnyComponent>&& fbcomponent = mfbs::CreateAnyComponent(builder, builder.CreateString(pComponent->GetTypeName()), fbdata);

		vFbComponents.push_back(fbcomponent);
	}

	auto&& fb_components = builder.CreateVector(vFbComponents);

	mfbs::MEntityBuilder entityBuilder(builder);

	entityBuilder.add_id(&fbguid);
	entityBuilder.add_name(fbname);
	entityBuilder.add_components(fb_components);

	return entityBuilder.Finish().Union();
}

void MEntity::Deserialize(const void* pBufferPointer)
{
	const mfbs::MEntity* fbEntity = reinterpret_cast<const mfbs::MEntity *>(pBufferPointer);

	const mfbs::MGuid* fbguid = fbEntity->id();
	m_id = MGuid(fbguid->data0(), fbguid->data1(), fbguid->data2(), fbguid->data3());
	m_strName = fbEntity->name()->c_str();

	for (auto&& fbcomponent : *fbEntity->components())
	{
		MString type = fbcomponent->type()->c_str();

		const MType* pType = MTypeClass::GetType(type);
		MComponent* pComponent = GetScene()->AddComponent(this, pType);
		pComponent->Deserialize(fbcomponent->data()->data());
	}
}

void MEntity::PostDeserialize()
{
	for (MComponentID& compid : m_vComponents)
	{
		if (MComponent* pComponent = GetScene()->GetComponent(compid))
		{
			pComponent->PostDeserialize();
		}
	}
}
