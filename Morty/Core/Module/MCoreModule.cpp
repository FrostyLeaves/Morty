#include "Module/MCoreModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"

#include "System/MComponentSystem.h"
#include "System/MEntitySystem.h"
#include "System/MConsoleSystem.h"
#include "System/MInputSystem.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MInputComponent.h"
#include "Component/MSceneComponent.h"

#include "Resource/MEntityResource.h"
#include "Resource/MResourceAsyncLoadSystem.h"

#include <iostream>

using namespace morty;

static void RegisterCoreCommands(MEngine* engine)
{
    auto* console = engine->GetSystem<MConsoleSystem>();
    if (!console) return;

    console->RegisterCommand(
            "scenes",
            "List all active scenes in the engine",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(args);
                auto* objectSystem = eng->GetSystem<MObjectSystem>();
                if (!objectSystem)
                {
                    std::cout << "ObjectSystem not found." << std::endl;
                    return false;
                }

                auto scenes = objectSystem->FindAllObjectsOfType<MScene>();

                std::cout << "Active Scenes:" << std::endl;
                if (scenes.empty()) { std::cout << "  (no scenes found)" << std::endl; }
                else
                {
                    for (size_t i = 0; i < scenes.size(); ++i)
                    {
                        auto* scene = scenes[i];
                        std::cout << "  [" << i << "] ObjectID: " << scene->GetObjectID()
                                  << ", Entities: " << scene->GetAllEntity().size()
                                  << std::endl;
                    }
                    std::cout << "Total: " << scenes.size() << " scene(s)" << std::endl;
                }

                return true;
            }
    );
}

bool MCoreModule::Register(MEngine* engine)
{
    if (!engine) return false;

    engine->RegisterSystem<MInputSystem>();
    engine->RegisterSystem<MConsoleSystem>();
    engine->RegisterSystem<MEntitySystem>();


    if (auto* objectSystem = engine->RegisterSystem<MObjectSystem>())
    {
        objectSystem->RegisterPostCreateObject(MCoreModule::OnObjectPostCreate);
    }

    if (auto* resourceSystem = engine->RegisterSystem<MResourceSystem>())
    {
#ifdef MORTY_RESOURCE_PATH
        resourceSystem->SetSearchPath({MORTY_RESOURCE_PATH});
#endif
        resourceSystem->RegisterResourceLoader<MEntityResourceLoader>();
    }

    engine->RegisterSystem<MResourceAsyncLoadSystem>();

    if (auto* pComponentSystem = engine->RegisterSystem<MComponentSystem>())
    {
        pComponentSystem->RegisterComponent<MSceneComponent>();
        pComponentSystem->RegisterComponent<MInputComponent>();
    }

    RegisterCoreCommands(engine);

    return true;
}

void MCoreModule::OnObjectPostCreate(MObject* pObject)
{
    if (!pObject) { return; }

    if (pObject->GetType() == MScene::GetClassType())
    {
        if (auto* scene = pObject->template DynamicCast<MScene>()) { scene->RegisterManager<MNotifyManager>(); }
    }
}
