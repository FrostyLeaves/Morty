#include "System/MSceneSystem.h"
#include "Scene/MEntity.h"

#include "Component/MSceneComponent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneSystem, MISystem)

MSceneSystem::MSceneSystem()
    : MISystem()
{}

MSceneSystem::~MSceneSystem() {}

void MSceneSystem::SetVisible(MEntity* pEntity, const bool& bVisible)
{
    MSceneComponent* sceneComponent = pEntity->GetComponent<MSceneComponent>();

    if (!sceneComponent) return;

    sceneComponent->SetVisible(bVisible);
}
