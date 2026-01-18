/**
 * @File         MEditorCommands
 *
 * @Created      2026-01-13
 *
 * @Author       DoubleYe
 **/

#include "Module/MEditorCommands.h"

#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Main/SDLRenderView.h"
#include "Scene/MScene.h"
#include "Tools/MModelImporter.h"

#include "System/MConsoleSystem.h"
#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

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
}

}// namespace morty
