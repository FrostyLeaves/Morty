#include "MEditorModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"

#include "System/MComponentSystem.h"
#include "System/MMoveControllerSystem.h"

#include "Component/MMoveControllerComponent.h"

using namespace morty;

bool MEditorModule::Register(MEngine* engine)
{
    if (!engine) return false;


    engine->RegisterSystem<MMoveControllerSystem>();

    if (MComponentSystem* pComponentSystem = engine->GetSystem<MComponentSystem>())
    {
        pComponentSystem->RegisterComponent<MMoveControllerComponent>();
    }

    return true;
}