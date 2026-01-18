#include "MEditorModule.h"
#include "Module/MEditorCommands.h"
#include "Engine/MEngine.h"

#include "Component/MMoveControllerComponent.h"
#include "System/MComponentSystem.h"
#include "System/MMoveControllerSystem.h"

using namespace morty;

bool MEditorModule::Register(MEngine* engine)
{
    if (!engine) return false;

    engine->RegisterSystem<MMoveControllerSystem>();

    if (MComponentSystem* pComponentSystem = engine->GetSystem<MComponentSystem>())
    {
        pComponentSystem->RegisterComponent<MMoveControllerComponent>();
    }

    RegisterEditorCommands(engine);

    return true;
}
