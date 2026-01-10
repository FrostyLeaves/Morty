#define DOCTEST_CONFIG_IMPLEMENT

#include "Engine/MEngine.h"
#include "MRenderModule.h"
#include "Module/MCoreModule.h"
#include "Module/MEditorModule.h"

#ifdef MORTY_WIN
#undef main
#endif

#include <iostream>

using namespace morty;

int main(int argc, char** argv)
{
    MORTY_UNUSED(argc);
    MORTY_UNUSED(argv);

    setvbuf(stdout, NULL, _IONBF, 0);

    MEngine engine;
    engine.Initialize();

    MCoreModule::Register(&engine);
    MRenderModule::Register(&engine);
    MEditorModule::Register(&engine);

    std::cout << "Morty Engine Console Mode" << std::endl;
    std::cout << "Type 'help' for available commands, 'exit' to quit." << std::endl;

    engine.Start();

    while (engine.IsRunning()) { engine.Update(); }

    engine.Stop();
    engine.Release();

    return 0;
}
