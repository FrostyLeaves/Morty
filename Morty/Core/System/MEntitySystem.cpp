#include "MEntitySystem.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MVariant.h"

#include "MSceneComponent.h"

#include "MEntityResource.h"

#include "MResourceSystem.h"

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
	MSceneComponent* pChildComp = pParent->GetComponent<MSceneComponent>();

	if (!pChildComp || !pParentComp)
		return;

	pChildComp->SetParentComponent(pParentComp->GetComponentID());
}

MResource* MEntitySystem::PackEntity(MEntity* pEntity)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	MEntityResource* pResource = pResourceSystem->CreateResource<MEntityResource>();

	pResource->m_entityStruct = MStruct();

	pResource->m_entityStruct = MStruct();
	pEntity->WriteToStruct(*pResource->m_entityStruct.GetStruct());

	return pResource;
}

MEntity* MEntitySystem::LoadEntity(MScene* pScene, MResource* pResource)
{
	MEntityResource* pEntityResource = pResource->DynamicCast<MEntityResource>();
	if (!pEntityResource)
		return nullptr;

	MEntity* pEntity = pScene->CreateEntity();
	if (!pEntity)
		return nullptr;

	if (MStruct* pStruct = pEntityResource->m_entityStruct.GetStruct())
	{
		pEntity->ReadFromStruct(*pStruct);
	}

	return pEntity;
}

