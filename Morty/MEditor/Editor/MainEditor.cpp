#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "MDirectX11Device.h"
#include "MEngine.h"
#include "MIRenderer.h"
#include "MIViewport.h"
#include "MResourceManager.h"
#include "MObject.h"
#include "MMaterialResource.h"
#include "MMaterial.h"
#include "MMeshInstance.h"
#include "MMesh.h"
#include "MIScene.h"

#include "Matrix.h"

MainEditor::MainEditor()
	: MWindowsRenderView()
	, m_pScene(nullptr)
{

}

MainEditor::~MainEditor()
{

}

void MainEditor::SetEditorNode(MNode* pNode)
{
	m_pScene->SetRootNode(pNode);
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


	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MIScene>();
	MIViewport* pViewport = m_pEngine->GetObjectManager()->CreateObject<MIViewport>();
	AppendViewport(pViewport);

	pViewport->SetScene(m_pScene);

	m_vViewport[0]->SetLeftTop(Vector2(100, 0));
	m_vViewport[0]->SetSize(Vector2(GetViewWidth() - 200, GetViewHeight()));

}

void MainEditor::Release()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	MWindowsRenderView::Release();
}

void MainEditor::OnResize(const int& nWidth, const int& nHeight)
{
	m_pEngine->GetRenderer()->OnResize(this, nWidth, nHeight);


	if (m_vViewport.empty())
		return;

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_vViewport[0]->SetLeftTop(Vector2(100, 0));
	m_vViewport[0]->SetSize(Vector2(nWidth - 200, nHeight));

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
		bool bMortyOpened = true;
		ImGui::SetNextItemOpen(bMortyOpened);
		ImGui::SetNextWindowSize(ImVec2(GetViewWidth(), GetViewHeight()));
		ImGui::Begin("Morty", &bMortyOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		bool bInfoOpened = true;

		ImGui::BeginChildFrame(ImGui::GetID("Morty"), ImVec2(150, 30), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

		ImGui::End();


		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
