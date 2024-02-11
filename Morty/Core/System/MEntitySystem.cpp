#include "System/MEntitySystem.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Variant/MVariant.h"

#include "Component/MSceneComponent.h"

#include "Resource/MEntityResource.h"

#include "System/MResourceSystem.h"

#include "flatbuffers/flatbuffer_builder.h"
#include "Flatbuffer/MEntityResource_generated.h"

using namespace morty;

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
	MORTY_ASSERT(pParent && pChild);

	if (!pParent || !pChild)
	{
		return;
	}

	MSceneComponent* pParentComp = pParent->GetComponent<MSceneComponent>();
	MSceneComponent* pChildComp = pChild->GetComponent<MSceneComponent>();

	MORTY_ASSERT(pParentComp && pChildComp);

	if (!pChildComp || !pParentComp)
	{
		return;
	}

	pChildComp->SetParentComponent(pParentComp->GetComponentID());
}

size_t GetSceneDepthFunction(MSceneComponent* pSceneComponent, std::map<MSceneComponent*, size_t>& tDepthCache)
{
	if (pSceneComponent == nullptr)
	{
		return 0;
	}

	if (tDepthCache.find(pSceneComponent) != tDepthCache.end())
	{
		return tDepthCache[pSceneComponent];
	}

	return tDepthCache[pSceneComponent] = GetSceneDepthFunction(pSceneComponent->GetParent(), tDepthCache) + 1;
}

std::shared_ptr<MResource> MEntitySystem::PackEntity(const std::vector<MEntity*>& vEntity)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MEntityResource> pResource = pResourceSystem->CreateResource<MEntityResource>();

	flatbuffers::FlatBufferBuilder fbb;

	std::map<MSceneComponent*, size_t> tDepthCache;
	std::vector<MEntity*> vSortEntity = vEntity;
	std::sort(vSortEntity.begin(), vSortEntity.end(), [&](MEntity* a, MEntity* b)
	{
		return GetSceneDepthFunction(a->GetComponent<MSceneComponent>(), tDepthCache) < GetSceneDepthFunction(b->GetComponent<MSceneComponent>(), tDepthCache);
	});

	std::vector<flatbuffers::Offset<fbs::MEntity>> entityVector;
	for (MEntity* pEntity : vSortEntity)
	{		
		flatbuffers::Offset<void> entity = pEntity->Serialize(fbb);
		entityVector.push_back(entity.o);
	}

	auto fb_entity = fbb.CreateVector(entityVector);

	fbs::MEntityResourceBuilder builder(fbb);

	builder.add_entity(fb_entity);

	flatbuffers::Offset<fbs::MEntityResource>&& root = builder.Finish();

	fbb.Finish(root);

	std::unique_ptr<MEntityResourceData> pEntityResourceData = std::make_unique<MEntityResourceData>();
	pEntityResourceData->aEntityData.resize(fbb.GetSize());
	memcpy(pEntityResourceData->aEntityData.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));
	pResource->Load(std::move(pEntityResourceData));

	return pResource;
}

std::vector<MEntity*> MEntitySystem::LoadEntity(MScene* pScene, std::shared_ptr<MResource> pResource)
{
	std::vector<MEntity*> vResult;

	MEntityResource* pEntityResource = pResource->template DynamicCast<MEntityResource>();
	if (!pEntityResource)
		return vResult;

	if (!pEntityResource->GetData() || !pEntityResource->GetSize())
	{
		GetEngine()->GetLogger()->Error("Load null entity resource.");
		return {};
	}

	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)pEntityResource->GetData(), pEntityResource->GetSize());

	const fbs::MEntityResource* fbResource = fbs::GetMEntityResource(fbb.GetCurrentBufferPointer());

	const flatbuffers::Vector<flatbuffers::Offset<fbs::MEntity>>& vEntity = *fbResource->entity();

	std::map<MGuid, MGuid> tRedirectGuid;
	tRedirectGuid[MGuid::invalid] = MGuid::invalid;
	for (size_t i = 0; i < vEntity.size(); ++i)
	{
		const fbs::MEntity* fb_entity = vEntity.Get(static_cast<flatbuffers::uoffset_t>(i));
		if (fb_entity->id())
		{
			MGuid fbGuid = MGuid(fb_entity->id()->data0(), fb_entity->id()->data1(), fb_entity->id()->data2(), fb_entity->id()->data3());
			tRedirectGuid[fbGuid] = MGuid::generate();
		}
	}

	for (size_t i = 0; i < vEntity.size(); ++i)
	{
		const fbs::MEntity* fb_entity = vEntity.Get(static_cast<flatbuffers::uoffset_t>(i));
		if (!fb_entity->id())
		{
			MORTY_ASSERT(fb_entity->id());
			continue;
		}

	    MGuid fbGuid = MGuid(fb_entity->id()->data0(), fb_entity->id()->data1(), fb_entity->id()->data2(), fb_entity->id()->data3());
		MGuid guid = tRedirectGuid[fbGuid];

		MEntity* pEntity = pScene->CreateEntity(guid);
		pEntity->Deserialize(fb_entity);

		vResult.push_back(pEntity);
	}

	for (size_t i = 0; i < vResult.size(); ++i)
	{
		vResult[i]->PostDeserialize(tRedirectGuid);
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
