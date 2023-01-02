﻿// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "SDL.h"
#include <fstream>

#include "Engine/MEngine.h"
#include "MainEditor.h"

#include "MRenderModule.h"

#include "Module/MEditorModule.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"
#include "System/MRenderSystem.h"

#include "Scene/MScene.h"

#ifdef MORTY_WIN
#undef main
#endif




#include "Test/ShadowMap.h"
#include "Test/EnvironmentCubemap.h"
#include "Test/LoadModel.h"



int main()
{
	//initialize
	MEngine engine;
	engine.Initialize();

	//register module
	MRenderModule::Register(&engine);
	MEditorModule::Register(&engine);


	bool bClosed = false;
	
	MainEditor editor;
	editor.Initialize(&engine, "Morty");
	editor.SetCloseCallback([&bClosed]() {

		bClosed = true;
		return true;
		});


	MScene* pScene = editor.GetScene();
	MORTY_ASSERT(pScene);


	//ENVIRONMENT_CUBEMAP_TEST(&engine, pScene);

	SHADOW_MAP_TEST(&engine, pScene);
	
	LOAD_MODEL_TEST(&engine, pScene);




	//start run
	engine.Start();

	while (!bClosed)
	{
		engine.Update();
	}

	editor.Release();

	//stop run
	engine.Stop();

	//release
	engine.Release();






}
