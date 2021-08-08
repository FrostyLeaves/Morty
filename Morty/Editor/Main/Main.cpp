// Demo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "MEngine.h"

#include "MRenderModule.h"

#include "MainEditor.h"

#include "SDL.h"
#include <fstream>

#include "MModelConverter.h"

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


	{
		MModelConverter convert(&engine);

		MModelConvertInfo info;
		info.eMaterialType = MModelConvertMaterialType::E_Default_Forward;
		info.strOutputDir = "D:/test";
		info.strOutputName = "cat";
		info.strResourcePath = "D:/project/Morty_Restructure/Resource/Model/cat.fbx";

		convert.Convert(info);
	}

	bool bClosed = false;
	
	MainEditor editor;
	editor.Initialize(&engine, "Morty");
	editor.SetCloseCallback([&bClosed]() {

		bClosed = true;
		return true;
		});

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
