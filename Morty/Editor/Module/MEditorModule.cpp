#include "MEditorModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"

#include "System/MComponentSystem.h"
#include "System/MMoveControllerSystem.h"

#include "Component/MMoveControllerComponent.h"

using namespace morty;

bool MEditorModule::Register(MEngine* pEngine)
{
    if (!pEngine) return false;


    pEngine->RegisterSystem<MMoveControllerSystem>();

    if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
    {
        pComponentSystem->RegisterComponent<MMoveControllerComponent>();
    }

    return true;
}