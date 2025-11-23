#define DOCTEST_CONFIG_IMPLEMENT


#include "Engine/MEngine.h"
#include "Main/MainEditor.h"

#include "MRenderModule.h"
#include "Module/MCoreModule.h"
#include "Module/MEditorModule.h"

#include "System/MEntitySystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Scene/MScene.h"

#ifdef MORTY_WIN
#undef main
#endif


#include "System/MObjectSystem.h"

using namespace morty;

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    //initialize
    MEngine engine;
    engine.Initialize();

    //register module
    MCoreModule::Register(&engine);
    MRenderModule::Register(&engine);
    MEditorModule::Register(&engine);

    //create window.
    SDLRenderView renderView;
    renderView.Initialize(&engine);

    //attach to window surface.
    renderView.BindSDLWindow();

    //create editor
    MainEditor editor;
    editor.Initialize(&engine);
    renderView.AppendContent(&editor);

    //create a scene.
    auto pScene = engine.FindSystem<MObjectSystem>()->CreateObject<MScene>();
    editor.SetScene(pScene);


    //start run
    engine.Start();

    while (!renderView.GetClosed()) { engine.Update(); }

    //stop run
    engine.Stop();

    //destroy editor
    editor.Release();

    //destroy window
    renderView.UnbindSDLWindow();
    renderView.Release();

    //release engine
    engine.Release();

    return 0;
}
