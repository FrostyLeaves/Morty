// MortyEditor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "MEngine.h"
#include "MWindowsRenderView.h"
#include "MDirectX11Renderer.h"

#include "MainEditor.h"

int _tmain(int argc, _TCHAR* argv[])
{
	MEngine engine;
	engine.Initialize();

	MDirectX11Renderer* pRenderer = dynamic_cast<MDirectX11Renderer*>(engine.GetRenderer());

	MainEditor* pMainEditor = new MainEditor();
	pMainEditor->Initialize(&engine, "Test");

	engine.AddView(pMainEditor);




	while (engine.MainLoop());
	
	
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	
	
	engine.Release();


	return 0;
}

