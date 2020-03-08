#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"


#include "MDirectX11Device.h"
#include "MEngine.h"
#include "MIRenderer.h"
#include "MViewport.h"
#include "MResourceManager.h"
#include "MObject.h"
#include "MMaterialResource.h"
#include "MMaterial.h"
#include "MIMeshInstance.h"
#include "MMesh.h"
#include "MScene.h"
#include "MCamera.h"
#include "MTransformCoord.h"
#include "MIRenderTarget.h"

#include "Matrix.h"

#include "NodeTreeView.h"
#include "PropertyView.h"

#include "MRenderStatistics.h"

MainEditor::MainEditor()
	: MWindowsRenderView()
	, m_pScene(nullptr)
	, m_pNodeTreeView(new NodeTreeView())
	, m_pPropertyView(new PropertyView())
{
	m_nWidth = 800.0f;
	m_nHeight = 480.0f;
}

MainEditor::~MainEditor()
{

}

void MainEditor::SetEditorNode(MNode* pNode)
{
	m_pScene->SetRootNode(pNode);
	m_pNodeTreeView->SetRootNode(pNode);
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


	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MScene>();
	MViewport* pViewport = m_pEngine->GetObjectManager()->CreateObject<MViewport>();
	AppendViewport(pViewport);

	pViewport->SetScene(m_pScene);

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
	MWindowsRenderView::OnResize(nWidth, nHeight);

	if (m_vViewport.empty())
		return;

	m_nWidth = nWidth;
	m_nHeight = nHeight;


}

void MainEditor::OnRenderEnd()
{
	static int nFps = 0;
	int nCurFps = (int)round(m_pEngine->GetInstantFPS() / 5) * 5;
	if (nFps != nCurFps)
	{
		nFps = nCurFps;

		char svWindowTitle[32];
		sprintf_s(svWindowTitle, "Morty FPS: %d", nFps);
		SetWindowTitle(svWindowTitle);
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	ImGuiStyle& style = ImGui::GetStyle();
	//ImGui::StyleColorsLight(&style);
	style.WindowRounding = 0.0f;
	style.WindowPadding = ImVec2(2.0f, 2.0f);
	style.ItemSpacing.x = 2.0f;

	ImVec4 bgColor = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_ChildWindowBg] = bgColor;
	
	{
		ImGuiWindowFlags unWindowFlags = 0;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		bool bMortyOpened = true;
		ImGui::SetNextItemOpen(bMortyOpened);
		ImGui::SetNextWindowSize(ImVec2(GetViewWidth(), GetViewHeight()));

		float fHeight = 0;
		if (ImGui::Begin("Morty", &bMortyOpened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize))
		{
			// 主窗口可用大小
			ImVec2 v2RegionAvail = ImGui::GetContentRegionAvail();

			fHeight = v2RegionAvail.y * 0.2f;

			v2RegionAvail.y -= (fHeight + 4.0f);
			
			//子窗口可用大小
			static float fNodeTreeWidth = v2RegionAvail.x * 0.25f;
			static float fPropertyWidth = v2RegionAvail.x * 0.4f;

			if (ImGui::BeginChild("NodeTree", ImVec2(fNodeTreeWidth, v2RegionAvail.y), true, unWindowFlags))
			{	
				m_pNodeTreeView->Render();
			}
			ImGui::EndChild();

			ImGui::SameLine();


			if (ImGui::BeginChild("Render", ImVec2(v2RegionAvail.x - fNodeTreeWidth - fPropertyWidth, v2RegionAvail.y), false, unWindowFlags | ImGuiWindowFlags_NoBackground))
			{
				ImVec2 v2RenderViewPos = ImGui::GetWindowPos();
				ImVec2 v2RenderViewSize = ImGui::GetWindowSize();

				m_vViewport[0]->SetLeftTop(Vector2(v2RenderViewPos.x, v2RenderViewPos.y));
				m_vViewport[0]->SetSize(Vector2(v2RenderViewSize.x, v2RenderViewSize.y));
			}
			ImGui::EndChild();


			ImGui::SameLine();

			if (ImGui::BeginChild("Property", ImVec2(fPropertyWidth, v2RegionAvail.y), true, unWindowFlags))
			{
				if (MNode * pNode = dynamic_cast<MNode*>(m_pNodeTreeView->GetSelectionNode()))
				{
					ImGui::Text(pNode->GetName().c_str());
				}
				m_pPropertyView->SetEditorObject(m_pNodeTreeView->GetSelectionNode());
				MNode* pNode = dynamic_cast<MNode*>(m_pNodeTreeView->GetSelectionNode());
				m_pScene->GetTransformCoord()->SetTarget3DNode(pNode);
				m_pPropertyView->Render();
			}
			ImGui::EndChild();
		}
		ImGui::BeginChild("Test", ImVec2(0, fHeight));
#if MORTY_RENDER_DATA_STATISTICS
		ImGui::Text("Vertex Count: %d", MRenderStatistics::GetInstance()->unVertexCount);
#endif
		ImGui::EndChild();


		ImGui::End();

	}

	
	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

void MainEditor::OnRenderBegin()
{	

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
