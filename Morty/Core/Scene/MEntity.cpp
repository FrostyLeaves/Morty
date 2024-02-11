#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Variant/MVariant.h"
#include "Utility/MFileHelper.h"
#include "System/MObjectSystem.h"

#include "Scene/MScene.h"
#include "Component/MComponent.h"

#include "Utility/MFunction.h"

#include "Flatbuffer/MEntity_generated.h"

using namespace morty;

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

MComponent* MEntity::RegisterComponent(const MType* pComponentType)
{
	if (!m_pScene)
	{
		return nullptr;
	}

	return m_pScene->AddComponent(this, pComponentType);
}

void MEntity::UnregisterComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return;

	m_pScene->RemoveComponent(this, pComponentType);
}

void MEntity::UnregisterAllComponent()
{
	auto components = m_tComponents;

	for (auto& pr : components)
	{
		UnregisterComponent(pr.first);
	}
}

bool MEntity::HasComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return false;

	return m_tComponents.find(pComponentType) != m_tComponents.end();
}

MComponent* MEntity::GetComponent(const MType* pComponentType)
{
	if (!m_pScene)
		return nullptr;

	auto findResult = m_tComponents.find(pComponentType);
	if (findResult != m_tComponents.end())
	{
		return findResult->second;
	}

	return nullptr;
}

std::vector<MComponent*> MEntity::GetComponents()
{
	std::vector<MComponent*> vResult;

	for (auto& pr : m_tComponents)
	{
		vResult.push_back(pr.second);
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
	fbs::MGuid fbguid(id.data[0], id.data[1], id.data[2], id.data[3]);
	auto fbname = builder.CreateString(GetName());

	std::vector<flatbuffers::Offset<fbs::AnyComponent >> vFbComponents;
	std::vector<MComponent*>&& vComponents = GetComponents();
	for (MComponent* pComponent : vComponents)
	{
		flatbuffers::FlatBufferBuilder compBuilder;
		flatbuffers::Offset<void>&& componentRoot = pComponent->Serialize(compBuilder);
		compBuilder.Finish(componentRoot);

		flatbuffers::Offset<flatbuffers::Vector<uint8_t>>&& fbdata = builder.CreateVector(compBuilder.GetBufferPointer(), compBuilder.GetSize());
		flatbuffers::Offset<fbs::AnyComponent>&& fbcomponent = fbs::CreateAnyComponent(builder, builder.CreateString(pComponent->GetTypeName()), fbdata);

		vFbComponents.push_back(fbcomponent);
	}

	auto fb_components = builder.CreateVector(vFbComponents);

	fbs::MEntityBuilder entityBuilder(builder);

	entityBuilder.add_id(&fbguid);
	entityBuilder.add_name(fbname);
	entityBuilder.add_components(fb_components);

	return entityBuilder.Finish().Union();
}

void MEntity::Deserialize(const void* pBufferPointer)
{
	const fbs::MEntity* fbEntity = reinterpret_cast<const fbs::MEntity *>(pBufferPointer);

	//const fbs::MGuid * fbguid = fbEntity->id();
	//m_id = MGuid(fbguid->data0(), fbguid->data1(), fbguid->data2(), fbguid->data3());
	m_strName = fbEntity->name()->c_str();

	const flatbuffers::Vector<flatbuffers::Offset<fbs::AnyComponent>>& vfbcomponents = *fbEntity->components();

	for (size_t i = 0; i < vfbcomponents.size(); ++i)
	{
		auto fbcomponent = vfbcomponents.Get(static_cast<flatbuffers::uoffset_t>(i));

		MString type = fbcomponent->type()->c_str();

		const MType* pType = MTypeClass::GetType(type);
		MComponent* pComponent = GetScene()->AddComponent(this, pType);

		flatbuffers::FlatBufferBuilder fbb;
		fbb.PushBytes(fbcomponent->data()->data(), fbcomponent->data()->size());
		pComponent->Deserialize(fbb);
	}
}

void MEntity::PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid)
{
	for (auto& pr : m_tComponents)
	{
		if (MComponent* pComponent = pr.second)
		{
			pComponent->PostDeserialize(tRedirectGuid);
		}
	}
}
