#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "MDirectX11Device.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MObject.h"
#include "MMaterialResource.h"
#include "MMaterial.h"
#include "MMeshInstance.h"
#include "MMesh.h"

#include "Matrix.h"

MainEditor::MainEditor()
{

}

MainEditor::~MainEditor()
{

}

bool MainEditor::Initialize(MEngine* pEngine, const char* svWindowName)
{

	if (!MWindowsRenderView::Initialize(pEngine, svWindowName))
		return false;



	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	MDirectX11Device* pDevice = dynamic_cast<MDirectX11Device*>(m_pEngine->GetDevice());

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(GetHWND());
	ImGui_ImplDX11_Init(pDevice->m_pD3dDevice, pDevice->m_pD3dContext);


}

void MainEditor::Release()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	MWindowsRenderView::Release();
}

void MainEditor::OnRenderEnd()
{

	// Our state
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(GetViewWidth(), GetViewHeight()));
		ImGui::Begin("Morty");





		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

Vector2 MainEditor::GetRenderRectTopLeft()
{
	return Vector2(0 + 100, 0);
}

Vector2 MainEditor::GetRenderRectSize()
{
	return Vector2(m_nWidth - 200, m_nHeight);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
