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
	MSceneComponent* pChildComp = pChild->GetComponent<MSceneComponent>();

	if (!pChildComp || !pParentComp)
		return;

	pChildComp->SetParentComponent(pParentComp->GetComponentID());
}

MResource* MEntitySystem::PackEntity(MScene* pScene, const std::vector<MEntity*>& vEntity)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	MEntityResource* pResource = pResourceSystem->CreateResource<MEntityResource>();

	pResource->m_entityStruct = MVariantArray();

	MComponentRefTable refTable(pScene);

	for (MEntity* pEntity : vEntity)
	{
		if (MStruct* pStruct = pResource->m_entityStruct.GetArray()->AppendMVariant<MStruct>())
		{
			pEntity->WriteToStruct(*pStruct, refTable);
		}
	}

	return pResource;
}

std::vector<MEntity*> MEntitySystem::LoadEntity(MScene* pScene, MResource* pResource)
{
	std::vector<MEntity*> vResult;

	MEntityResource* pEntityResource = pResource->DynamicCast<MEntityResource>();
	if (!pEntityResource)
		return vResult;


	if (const MVariantArray* pArray = pEntityResource->m_entityStruct.GetArray())
	{
		MComponentRefTable refTable(pScene);

		for (size_t i = 0; i < pArray->GetMemberCount(); ++i)
		{
			if (const MStruct* pStruct = pArray->GetMember<MStruct>(i))
			{
				if (MEntity* pEntity = pScene->CreateEntity())
				{
					pEntity->ReadFromStruct(*pStruct, refTable);
					vResult.push_back(pEntity);
				}
			}
		}

		refTable.BindReference();
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
