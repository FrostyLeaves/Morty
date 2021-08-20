// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "SDL.h"
#include <fstream>

#include "MEngine.h"
#include "MainEditor.h"

#include "MRenderModule.h"
#include "MModelConverter.h"

#include "MEditorModule.h"

#include "MEntitySystem.h"
#include "MResourceSystem.h"

#include "MScene.h"
#include "MSceneComponent.h"
#include "MDirectionalLightComponent.h"

#ifdef MORTY_WIN
#undef main
#endif

int main()
{
	//initialize
	MEngine engine;
	engine.Initialize();

	//register module
	MRenderModule::Register(&engine);
	MEditorModule::Register(&engine);


// 	{
// 		MModelConverter convert(&engine);
// 
// 		MModelConvertInfo info;
// 		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
// 		info.strOutputDir = "D:/test";
// 		info.strOutputName = "banana";
// 		info.strResourcePath = "D:/project/Morty_Restructure/Resource/Model/banana/ripe-banana.obj";
// 
// 		convert.Convert(info);
// 	}

	bool bClosed = false;
	
	MainEditor editor;
	editor.Initialize(&engine, "Morty");
	editor.SetCloseCallback([&bClosed]() {

		bClosed = true;
		return true;
		});

	if (MScene* pScene = editor.GetScene())
	{
		MResourceSystem* pResourceSystem = engine.FindSystem<MResourceSystem>();
		MEntitySystem* pEntitySystem = engine.FindSystem<MEntitySystem>();

		
		MResource* pResource = pResourceSystem->LoadResource("D:/test/banana/banana.entity");

		for (size_t i = 0; i < 2; ++i)
		{
			pEntitySystem->LoadEntity(pScene, pResource);
		}

		MEntity* pDirLight = pScene->CreateEntity();
		pDirLight->SetName("DirectionalLight");
		if (MSceneComponent* pSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
		{
			//pSceneComponent->SetRotation(Quaternion(Vector3(1.0, 0.0, 0.0), 45.0f));
		}
		pDirLight->RegisterComponent<MDirectionalLightComponent>();



	}

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
