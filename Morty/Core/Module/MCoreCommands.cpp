/**
 * @File         MCoreCommands
 *
 * @Created      2026-01-13
 *
 * @Author       DoubleYe
 **/

#include "Module/MCoreCommands.h"

#include "Engine/MEngine.h"
#include "Scene/MScene.h"
#include "System/MConsoleSystem.h"
#include "System/MObjectSystem.h"

#include <iostream>

namespace morty
{

static MScene* FindSceneByName(MObjectSystem* objectSystem, const MStringId& sceneName)
{
    if (!objectSystem) { return nullptr; }

    auto scenes = objectSystem->FindAllObjectsOfType<MScene>();
    for (MScene* scene: scenes)
    {
        if (scene && scene->GetName() == sceneName) { return scene; }
    }

    return nullptr;
}

void RegisterCoreCommands(MEngine* engine)
{
    auto* console = engine->GetSystem<MConsoleSystem>();
    if (!console) return;

    console->RegisterCommand(
            "scene",
            "Manage scenes. Usage: scene <list|create|delete> [name]",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                auto* objectSystem = eng->GetSystem<MObjectSystem>();
                if (!objectSystem)
                {
                    std::cout << "ObjectSystem not found." << std::endl;
                    return false;
                }

                if (args.size() < 2)
                {
                    std::cout << "Usage: scene <list|create|delete> [name]" << std::endl;
                    return false;
                }

                const MString& action = args[1];
                if (action == "list")
                {
                    auto scenes = objectSystem->FindAllObjectsOfType<MScene>();

                    std::cout << "Active Scenes:" << std::endl;
                    if (scenes.empty()) { std::cout << "  (no scenes found)" << std::endl; }
                    else
                    {
                        for (size_t i = 0; i < scenes.size(); ++i)
                        {
                            auto* scene = scenes[i];
                            const char* sceneName = scene->GetName().empty() ? "(empty)" : scene->GetName().c_str();
                            std::cout << "  [" << i << "] Name: " << sceneName
                                      << ", ObjectID: " << scene->GetObjectID()
                                      << ", Entities: " << scene->GetAllEntity().size() << std::endl;
                        }
                        std::cout << "Total: " << scenes.size() << " scene(s)" << std::endl;
                    }

                    return true;
                }
                else if (action == "create")
                {
                    if (args.size() < 3)
                    {
                        std::cout << "Usage: scene create <name>" << std::endl;
                        return false;
                    }

                    const MStringId sceneName(args[2]);
                    MScene* scene = objectSystem->CreateObject<MScene>();
                    if (!scene)
                    {
                        std::cout << "Failed to create scene." << std::endl;
                        return false;
                    }

                    scene->SetName(sceneName);
                    std::cout << "Created scene: " << sceneName.c_str()
                              << " (ObjectID: " << scene->GetObjectID() << ")" << std::endl;
                    return true;
                }
                else if (action == "delete")
                {
                    if (args.size() < 3)
                    {
                        std::cout << "Usage: scene delete <name>" << std::endl;
                        return false;
                    }

                    const MStringId sceneName(args[2]);
                    MScene*         scene = FindSceneByName(objectSystem, sceneName);
                    if (!scene)
                    {
                        std::cout << "Scene with name " << sceneName.c_str() << " not found." << std::endl;
                        return false;
                    }

                    const MObjectID sceneId = scene->GetObjectID();
                    scene->DeleteLater();
                    std::cout << "Scene " << sceneName.c_str() << " (ObjectID: " << sceneId
                              << ") marked for deletion." << std::endl;
                    return true;
                }

                std::cout << "Usage: scene <list|create|delete> [name]" << std::endl;
                return false;
            }
    );
}

}// namespace morty
