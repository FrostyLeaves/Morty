#define DOCTEST_CONFIG_IMPLEMENT


#include "Engine/MEngine.h"
#include "MRenderModule.h"
#include "Main/MainEditor.h"
#include "Module/MCoreModule.h"
#include "Module/MEditorModule.h"
#include "Scene/MScene.h"
#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Tools/MModelImporter.h"

#ifdef MORTY_WIN
#undef main
#endif

#include <CLI/CLI.hpp>
#include <iostream>
#include <thread>

using namespace morty;

void RunEditor(MEngine* engine)
{

    //create window.
    SDLRenderView renderView;
    renderView.Initialize(engine);

    //attach to window surface.
    renderView.BindSDLWindow();

    //create editor
    MainEditor editor;
    editor.Initialize(engine);
    renderView.AppendContent(&editor);

    //create a scene.
    auto scene = engine->FindSystem<MObjectSystem>()->CreateObject<MScene>();
    editor.SetScene(scene);


    //start run
    engine->Start();

    while (!renderView.GetClosed()) { engine->Update(); }

    //stop run
    engine->Stop();

    //destroy editor
    editor.Release();

    //destroy window
    renderView.UnbindSDLWindow();
    renderView.Release();
}

int main(int argc, char** argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    //initialize
    MEngine engine;
    engine.Initialize();

    //register module
    MCoreModule::Register(&engine);
    MRenderModule::Register(&engine);
    MEditorModule::Register(&engine);


    CLI::App app{"Morty Engine CLI"};
    app.require_subcommand(0, 1);

    // Editor command
    auto editor_cmd = app.add_subcommand("editor", "Start the Morty editor");
    editor_cmd->callback([&]() {
        std::cout << "Starting Morty Editor...\n";
        RunEditor(&engine);
    });

    // Import subcommand
    auto        import_cmd = app.add_subcommand("import", "Import resources");

    // Import model command
    std::string model_input_path;
    std::string model_output_path;
    auto        import_model_cmd = import_cmd->add_subcommand("model", "Import a model file");
    import_model_cmd->add_option("input", model_input_path, "Input model file")->required()->check(CLI::ExistingFile);
    import_model_cmd->add_option("output", model_output_path, "Output path")->required();
    import_model_cmd->callback([&]() {
        std::cout << "Import model: " << model_input_path << "\n"
                  << "Output to: " << model_output_path << "\n";

        MModelConvertInfo info;
        info.strResourcePath = model_input_path;
        info.strOutputDir    = model_output_path;
        info.bImportCamera   = false;
        info.bImportLights   = false;

        MModelImporter importer(&engine);
        bool           success = importer.Import(info);
        if (success) { std::cout << "Model imported successfully.\n"; }
        else { std::cout << "Failed to import model.\n"; }
    });

    import_cmd->require_subcommand(1);

    // Parse command line arguments
    try
    {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e)
    {
        return app.exit(e);
    }


    //release engine
    engine.Release();

    return 0;
}
