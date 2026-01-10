#include "MEditorModule.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Main/SDLRenderView.h"
#include "Resource/MEntityResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Tools/MModelImporter.h"

#include "Component/MMoveControllerComponent.h"
#include "System/MComponentSystem.h"
#include "System/MConsoleSystem.h"
#include "System/MEntitySystem.h"
#include "System/MMoveControllerSystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include <iostream>

using namespace morty;

static void RunEditor(MEngine* engine)
{
    SDLRenderView renderView;
    renderView.Initialize(engine);
    renderView.BindSDLWindow();

    MainEditor editor;
    editor.Initialize(engine);
    renderView.AppendContent(&editor);

    auto scene = engine->GetSystem<MObjectSystem>()->CreateObject<MScene>();
    editor.SetScene(scene);

    while (!renderView.GetClosed()) { engine->Update(); }

    editor.Release();

    renderView.UnbindSDLWindow();
    renderView.Release();
}

static void RegisterEditorCommands(MEngine* engine)
{
    auto* console = engine->GetSystem<MConsoleSystem>();
    if (!console) return;

    console->RegisterCommand(
            "editor",
            "Start the Morty editor",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                MORTY_UNUSED(args);
                RunEditor(eng);
                return true;
            }
    );

    console->RegisterCommand(
            "convert",
            "Convert a model file to engine format. Usage: convert <input> <output>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 3)
                {
                    std::cout << "Usage: convert <input_path> <output_path>" << std::endl;
                    return false;
                }

                std::cout << "Converting model: " << args[1] << std::endl;
                std::cout << "Output to: " << args[2] << std::endl;

                MModelConvertInfo info;
                info.strResourcePath = args[1];
                info.strOutputDir    = args[2];
                info.bImportCamera   = false;
                info.bImportLights   = false;

                MModelImporter importer(eng);
                bool           success = importer.Import(info);
                if (success) { std::cout << "Model converted successfully." << std::endl; }
                else { std::cout << "Failed to convert model." << std::endl; }
                return success;
            }
    );

    console->RegisterCommand(
            "load",
            "Load an entity resource into a scene. Usage: load <entity_path> <scene_object_id>",
            [](MEngine* eng, const std::vector<MString>& args) -> bool {
                if (args.size() < 3)
                {
                    std::cout << "Usage: load <entity_path> <scene_object_id>" << std::endl;
                    return false;
                }

                const MString& entityPath   = args[1];
                MObjectID      sceneId      = static_cast<MObjectID>(std::stoul(args[2]));

                auto* objectSystem   = eng->GetSystem<MObjectSystem>();
                auto* resourceSystem = eng->GetSystem<MResourceSystem>();
                auto* entitySystem   = eng->GetSystem<MEntitySystem>();

                if (!objectSystem || !resourceSystem || !entitySystem)
                {
                    std::cout << "Required system not found." << std::endl;
                    return false;
                }

                MObject* obj = objectSystem->FindObject(sceneId);
                if (!obj || obj->GetType() != MScene::GetClassType())
                {
                    std::cout << "Scene with ObjectID " << sceneId << " not found." << std::endl;
                    return false;
                }

                MScene* scene = obj->DynamicCast<MScene>();

                std::cout << "Loading entity: " << entityPath << std::endl;
                std::cout << "Into scene: " << sceneId << std::endl;

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