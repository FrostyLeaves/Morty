#include "MSceneSystem.h"
#include "MEntity.h"

#include "MSceneComponent.h"

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
