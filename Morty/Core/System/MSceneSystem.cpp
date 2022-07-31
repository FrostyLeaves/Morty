#include "System/MSceneSystem.h"
#include "Scene/MEntity.h"

#include "Component/MSceneComponent.h"

MORTY_CLASS_IMPLEMENT(MSceneSystem, MISystem)

MSceneSystem::MSceneSystem()
	: MISystem()
{

}

MSceneSystem::~MSceneSystem()
{

}

void MSceneSystem::SetVisible(MEntity* pEntity, const bool& bVisible)
{
	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();

	if (!pSceneComponent)
		return;

	pSceneComponent->SetVisible(bVisible);
}
