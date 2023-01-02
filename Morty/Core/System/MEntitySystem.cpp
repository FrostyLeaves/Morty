#include "System/MEntitySystem.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Utility/MVariant.h"

#include "Component/MSceneComponent.h"

#include "Resource/MEntityResource.h"

#include "System/MResourceSystem.h"

#include "flatbuffers/flatbuffer_builder.h"
#include "Flatbuffer/MEntityResource_generated.h"

MORTY_CLASS_IMPLEMENT(MEntitySystem, MISystem)

MEntitySystem::MEntitySystem()
	: MISystem()
{

}

MEntitySystem::~MEntitySystem()
{

}

void MEntitySystem::AddChild(MEntity* pParent, MEntity* pChild)
{
	if (!pParent || !pChild)
		return;

	MSceneComponent* pParentComp = pParent->GetComponent<MSceneComponent>();
	MSceneComponent* pChildComp = pChild->GetComponent<MSceneComponent>();

	if (!pChildComp || !pParentComp)
		return;

	pChildComp->SetParentComponent(pParentComp->GetComponentID());
}

std::shared_ptr<MResource> MEntitySystem::PackEntity(MScene* pScene, const std::vector<MEntity*>& vEntity)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MEntityResource> pResource = pResourceSystem->CreateResource<MEntityResource>();

	flatbuffers::FlatBufferBuilder fbb;

	std::vector<flatbuffers::Offset<mfbs::MEntity>> entityVector;
	for (MEntity* pEntity : vEntity)
	{		
		flatbuffers::Offset<void>&& entity = pEntity->Serialize(fbb);
		entityVector.push_back(entity.o);
	}

	auto&& fb_entity = fbb.CreateVector(entityVector);

	mfbs::MEntityResourceBuilder builder(fbb);

	builder.add_entity(fb_entity);

	flatbuffers::Offset<mfbs::MEntityResource>&& root = builder.Finish();

	fbb.Finish(root);

	pResource->GetData().clear();
	pResource->GetData().resize(fbb.GetSize());
	memcpy(pResource->GetData().data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

	return pResource;
}

std::vector<MEntity*> MEntitySystem::LoadEntity(MScene* pScene, std::shared_ptr<MResource> pResource)
{
	std::vector<MEntity*> vResult;

	MEntityResource* pEntityResource = pResource->DynamicCast<MEntityResource>();
	if (!pEntityResource)
		return vResult;


	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)pEntityResource->GetData().data(), pEntityResource->GetData().size());

	const mfbs::MEntityResource* fbResource = mfbs::GetMEntityResource(fbb.GetCurrentBufferPointer());

	const flatbuffers::Vector<flatbuffers::Offset<mfbs::MEntity>>& vEntity = *fbResource->entity();

	for (int i = 0; i < vEntity.size(); ++i)
	{
		MGuid guid = MGuid::generate();

		const mfbs::MEntity* fb_entity = vEntity.Get(i);
		if (fb_entity->id())
		{
			guid = MGuid(fb_entity->id()->data0(), fb_entity->id()->data1(), fb_entity->id()->data2(), fb_entity->id()->data3());
		}

		MEntity* pEntity = pScene->CreateEntity(guid);
		pEntity->Deserialize(fb_entity);

		vResult.push_back(pEntity);
	}

	for (int i = 0; i < vResult.size(); ++i)
	{
		vResult[i]->PostDeserialize();
	}

	
	return vResult;
}

void MEntitySystem::FindAllComponentRecursively(MEntity* pEntity, const MType* pComponentType, std::vector<MComponentID>& vResult)
{
	if (!pEntity)
		return;

	MScene* pScene = pEntity->GetScene();
	if (!pScene)
		return;

	if (MComponent* pFindResult = pEntity->GetComponent(pComponentType))
	{
		vResult.push_back(pFindResult->GetComponentID());
	}

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return;

	for (const MComponentID& childID : pSceneComponent->GetChildrenComponent())
	{
		if (MComponent* pChildComponent = pScene->GetComponent(childID))
		{
			if (MEntity* pChildEntity = pChildComponent->GetEntity())
			{
				FindAllComponentRecursively(pChildEntity, pComponentType, vResult);
			}
		}
	}
}
