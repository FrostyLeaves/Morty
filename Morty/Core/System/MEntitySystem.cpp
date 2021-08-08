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

void MEntitySystem::LoadEntity(MScene* pScene, MResource* pResource)
{
	MEntityResource* pEntityResource = pResource->DynamicCast<MEntityResource>();
	if (!pEntityResource)
		return;


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
				}
			}
		}

		refTable.BindReference();
	}

}

