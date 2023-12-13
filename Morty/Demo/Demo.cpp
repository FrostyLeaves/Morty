#define DOCTEST_CONFIG_IMPLEMENT

#include "SDL.h"
#include <fstream>

#include "Engine/MEngine.h"
#include "Main/MainEditor.h"

#include "Module/MCoreModule.h"
#include "MRenderModule.h"
#include "Module/MEditorModule.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "System/MRenderSystem.h"

#include "Scene/MScene.h"

#ifdef MORTY_WIN
#undef main
#endif


#include "System/MObjectSystem.h"
#include "Test/ShadowMap.h"
#include "Test/EnvironmentCubemap.h"
#include "Test/GPUDrivenCulling.h"
#include "Test/LoadModel.h"
#include "Test/LoadModel_Sponza.h"
#include "Test/Pbr.h"
#include "Test/Floor.h"
#include "Test/BasicTransform.h"
#include "Test/VXGI.h"

int main()
{
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
	MScene* pScene = engine.FindSystem<MObjectSystem>()->CreateObject<MScene>();
	editor.SetScene(pScene);

	ADD_DIRECTIONAL_LIGHT(&engine, pScene);
	//CREATE_FLOOR_GRID(&engine, pScene);
	//ENVIRONMENT_CUBEMAP_TEST(&engine, pScene);
	//SHADOW_MAP_TEST(&engine, pScene);
	//PBR_SHPERE(&engine, pScene);
	//LOAD_MODEL_ANIMATION_TEST(&engine, pScene);
	//	LOAD_MODEL_TRANSLATION_TEST(&engine, pScene);
	//LOAD_MODEL_SPONZA_TEST(&engine, pScene);
	//	GPU_DRIVEN_CULLING_TEST(&engine, pScene);
	//	TRANSFORM_SPHERE_GENERATE(&engine, pScene);
	VXGI_TEST(&engine, pScene);


	//start run
	engine.Start();

	while (!renderView.GetClosed())
	{
		engine.Update();
	}

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
