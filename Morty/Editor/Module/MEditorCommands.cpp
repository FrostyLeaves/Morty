/**
 * @File         MEditorCommands
 *
 * @Created      2026-01-13
 *
 * @Author       DoubleYe
 **/

#include "Module/MEditorCommands.h"

#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Main/SDLRenderView.h"
#include "Main/SimpleSceneContent.h"
#include "Module/MPropertyAccessor.h"
#include "Scene/MScene.h"
#include "Tools/MModelImporter.h"

#include "System/MConsoleSystem.h"
#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include <iostream>
#include <sstream>

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

static bool RunEditor(MEngine* engine, const MStringId& sceneName)
{
    auto* objectSystem = engine->GetSystem<MObjectSystem>();
    if (!objectSystem)
    {
        std::cout << "Object system not found." << std::endl;
        return false;
    }

    MScene* scene = FindSceneByName(objectSystem, sceneName);
    if (!scene)
    {
        std::cout << "Scene with name " << sceneName.c_str() << " not found." << std::endl;
        return false;
    }

    const MObjectID sceneId = scene->GetObjectID();

    SDLRenderView renderView;
    renderView.Initialize(engine);
    renderView.SetWindowTitle("Morty Editor");
    renderView.BindSDLWindow();

    MainEditor editor;
    editor.Initialize(engine);
    renderView.AppendContent(&editor);

    editor.SetScene(scene);

    while (objectSystem->FindObject(sceneId) && !renderView.GetClosed()) { engine->Update(); }

    editor.Release();

    renderView.UnbindSDLWindow();
    renderView.Release();
    return true;
}

static bool RunWindow(MEngine* engine, const MStringId& sceneName)
{
    auto* objectSystem = engine->GetSystem<MObjectSystem>();
    if (!objectSystem)
    {
        std::cout << "Object system not found." << std::endl;
        return false;
    }

    MScene* scene = FindSceneByName(objectSystem, sceneName);
    if (!scene)
    {
        std::cout << "Scene with name " << sceneName.c_str() << " not found." << std::endl;
        return false;
    }

    const MObjectID sceneId = scene->GetObjectID();

    SDLRenderView renderView;
    renderView.Initialize(engine);
    renderView.SetWindowTitle("Morty Viewer - " + MString(sceneName.c_str()));
    renderView.BindSDLWindow();

    SimpleSceneContent sceneContent;
    sceneContent.Initialize(engine, scene);
    renderView.AppendContent(&sceneContent);

    while (objectSystem->FindObject(sceneId) && !renderView.GetClosed()) { engine->Update(); }

    sceneContent.Release();

    renderView.UnbindSDLWindow();
    renderView.Release();
    return true;
}

void RegisterEditorCommands(MEngine* engine)
{
    auto* console = engine->GetSystem<MConsoleSystem>();
    if (!console) return;

    console->RegisterCommand(
            "editor",
            "Start the Morty editor. Usage: editor <scene_name>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 2)
                {
                    std::cout << "Usage: editor <scene_name>" << std::endl;
                    return false;
                }

                const MStringId sceneName(args[1]);
                return RunEditor(eng, sceneName);
            }
    );

    console->RegisterCommand(
            "window",
            "Display a scene in a simple window. Usage: window <scene_name>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 2)
                {
                    std::cout << "Usage: window <scene_name>" << std::endl;
                    return false;
                }

                const MStringId sceneName(args[1]);
                return RunWindow(eng, sceneName);
            }
    );

    console->RegisterCommand(
            "convert",
            "Convert a model file to engine format. Usage: convert <input> <output> [--nanite]",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 3)
                {
                    std::cout << "Usage: convert <input_path> <output_path> [--nanite]" << std::endl;
                    return false;
                }

                // Check for --nanite flag
                bool enableNanite = false;
                for (size_t i = 3; i < args.size(); ++i)
                {
                    if (args[i] == "--nanite") { enableNanite = true; }
                }

                std::cout << "Converting model: " << args[1] << std::endl;
                std::cout << "Output to: " << args[2] << std::endl;
                if (enableNanite) { std::cout << "Nanite enabled" << std::endl; }

                MModelConvertInfo info;
                info.strResourcePath = args[1];
                info.strOutputDir    = args[2];
                info.bImportCamera   = false;
                info.bImportLights   = false;
                info.bEnableNanite   = enableNanite;

                MModelImporter importer(eng);
                bool           success = importer.Import(info);
                if (success) { std::cout << "Model converted successfully." << std::endl; }
                else { std::cout << "Failed to convert model." << std::endl; }
                return success;
            }
    );

    console->RegisterCommand(
            "load",
            "Load an entity resource into a scene. Usage: load <entity_path> <scene_name>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 3)
                {
                    std::cout << "Usage: load <entity_path> <scene_name>" << std::endl;
                    return false;
                }

                const MString& entityPath = args[1];
                const MStringId sceneName(args[2]);

                auto* objectSystem   = eng->GetSystem<MObjectSystem>();
                auto* resourceSystem = eng->GetSystem<MResourceSystem>();
                auto* entitySystem   = eng->GetSystem<MEntitySystem>();

                if (!objectSystem || !resourceSystem || !entitySystem)
                {
                    std::cout << "Required system not found." << std::endl;
                    return false;
                }

                MScene* scene = FindSceneByName(objectSystem, sceneName);
                if (!scene)
                {
                    std::cout << "Scene with name " << sceneName.c_str() << " not found." << std::endl;
                    return false;
                }

                std::cout << "Loading entity: " << entityPath << std::endl;
                std::cout << "Into scene: " << sceneName.c_str()
                          << " (ObjectID: " << scene->GetObjectID() << ")" << std::endl;

                auto resource = resourceSystem->LoadResource(entityPath);
                if (!resource)
                {
                    std::cout << "Failed to load entity resource: " << entityPath << std::endl;
                    return false;
                }

                auto entities = entitySystem->LoadEntity(scene, resource);
                if (entities.empty())
                {
                    std::cout << "Failed to create entities from resource." << std::endl;
                    return false;
                }

                std::cout << "Loaded " << entities.size() << " entity(s) successfully." << std::endl;
                return true;
            }
    );

    console->RegisterCommand(
            "get",
            "Get a component property. Usage: get <scene_name> <path:Component:Property>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 3)
                {
                    std::cout << "Usage: get <scene_name> <path:Component:Property>" << std::endl;
                    std::cout << "Example: get MainScene root/child:MSceneComponent:Visible" << std::endl;
                    return false;
                }

                const MStringId sceneName(args[1]);
                auto*           objectSystem = eng->GetSystem<MObjectSystem>();
                MScene*         scene        = FindSceneByName(objectSystem, sceneName);

                if (!scene)
                {
                    std::cout << "Scene not found: " << args[1] << std::endl;
                    return false;
                }

                auto path = MPropertyAccessor::ParsePath(args[2]);
                if (!path.valid)
                {
                    std::cout << "Invalid path format. Use: path/to/entity:ComponentName:PropertyName"
                              << std::endl;
                    return false;
                }

                auto result = MPropertyAccessor::GetProperty(scene, path);
                if (!result.success)
                {
                    std::cout << "Error: " << result.errorMessage << std::endl;
                    return false;
                }

                std::cout << path.propertyName << " = " << result.value << std::endl;
                return true;
            }
    );

    console->RegisterCommand(
            "set",
            "Set a component property. Usage: set <scene_name> <path:Component:Property> <value>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 4)
                {
                    std::cout << "Usage: set <scene_name> <path:Component:Property> <value>" << std::endl;
                    std::cout << "Example: set MainScene root/child:MSceneComponent:Visible true" << std::endl;
                    return false;
                }

                const MStringId sceneName(args[1]);
                auto*           objectSystem = eng->GetSystem<MObjectSystem>();
                MScene*         scene        = FindSceneByName(objectSystem, sceneName);

                if (!scene)
                {
                    std::cout << "Scene not found: " << args[1] << std::endl;
                    return false;
                }

                auto path = MPropertyAccessor::ParsePath(args[2]);
                if (!path.valid)
                {
                    std::cout << "Invalid path format. Use: path/to/entity:ComponentName:PropertyName"
                              << std::endl;
                    return false;
                }

                // Join remaining args as value (to support values with spaces)
                MString value = args[3];
                for (size_t i = 4; i < args.size(); ++i) { value += " " + args[i]; }

                auto result = MPropertyAccessor::SetProperty(scene, path, value);
                if (!result.success)
                {
                    std::cout << "Error: " << result.errorMessage << std::endl;
                    return false;
                }

                std::cout << "Property set successfully." << std::endl;
                return true;
            }
    );

    console->RegisterCommand(
            "ls",
            "List child entities. Usage: ls <scene_name> [path/to/entity]",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 2)
                {
                    std::cout << "Usage: ls <scene_name> [path/to/entity]" << std::endl;
                    std::cout << "Example: ls MainScene" << std::endl;
                    std::cout << "Example: ls MainScene root/child" << std::endl;
                    return false;
                }

                const MStringId sceneName(args[1]);
                auto*           objectSystem = eng->GetSystem<MObjectSystem>();
                MScene*         scene        = FindSceneByName(objectSystem, sceneName);

                if (!scene)
                {
                    std::cout << "Scene not found: " << args[1] << std::endl;
                    return false;
                }

                // If no path specified, list root entities
                if (args.size() < 3)
                {
                    auto entities = scene->GetAllEntity();
                    std::cout << "Root entities in scene '" << args[1] << "':" << std::endl;
                    for (MEntity* entity: entities)
                    {
                        if (!entity) continue;
                        auto* sceneComp = entity->GetComponent<MSceneComponent>();
                        // Check if it's a root (no parent)
                        if (!sceneComp || !sceneComp->GetParentComponent().IsValid())
                        {
                            std::cout << "  " << entity->GetName() << std::endl;
                        }
                    }
                    return true;
                }

                // Parse path and find entity
                std::vector<MString> nodePath;
                std::stringstream    pathSs(args[2]);
                MString              nodePart;
                while (std::getline(pathSs, nodePart, '/'))
                {
                    if (!nodePart.empty()) { nodePath.push_back(nodePart); }
                }

                MEntity* entity = MPropertyAccessor::FindEntityByPath(scene, nodePath);
                if (!entity)
                {
                    std::cout << "Entity not found at path: " << args[2] << std::endl;
                    return false;
                }

                auto* sceneComp = entity->GetComponent<MSceneComponent>();
                if (!sceneComp)
                {
                    std::cout << "Entity '" << entity->GetName() << "' has no MSceneComponent (no children)"
                              << std::endl;
                    return true;
                }

                const auto& children = sceneComp->GetChildrenComponent();
                if (children.empty())
                {
                    std::cout << "Entity '" << entity->GetName() << "' has no children." << std::endl;
                    return true;
                }

                std::cout << "Children of '" << entity->GetName() << "':" << std::endl;
                for (const auto& childId: children)
                {
                    if (auto* childComp = static_cast<MSceneComponent*>(scene->GetComponent(childId)))
                    {
                        if (MEntity* childEntity = childComp->GetEntity())
                        {
                            std::cout << "  " << childEntity->GetName() << std::endl;
                        }
                    }
                }
                return true;
            }
    );
}

}// namespace morty
