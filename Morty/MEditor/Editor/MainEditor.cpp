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
#include "MCamera.h"
#include "MTransformCoord.h"

#include "Matrix.h"

#include "NodeTreeView.h"
#include "PropertyView.h"

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


	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MIScene>();
	MIViewport* pViewport = m_pEngine->GetObjectManager()->CreateObject<MIViewport>();
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
	m_pEngine->GetRenderer()->OnResize(this, nWidth, nHeight);


	if (m_vViewport.empty())
		return;

	m_nWidth = nWidth;
	m_nHeight = nHeight;


}

void MainEditor::OnRenderBegin()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


	char svWindowTitle[32];
	sprintf_s(svWindowTitle, "Morty FPS: %d", (int)round(m_pEngine->GetInstantFPS()));
	SetWindowTitle(svWindowTitle);


	ImGuiStyle& style = ImGui::GetStyle();
//	ImGui::StyleColorsLight(&style);
	style.WindowRounding = 0.0f;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		ImGuiWindowFlags unWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		bool bMortyOpened = true;
		ImGui::SetNextItemOpen(bMortyOpened);
		ImGui::SetNextWindowSize(ImVec2(GetViewWidth(), GetViewHeight()));
		if (ImGui::Begin("Morty", &bMortyOpened, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			// 主窗口可用大小
			ImVec2 v2RegionAvail = ImGui::GetContentRegionAvail();
			v2RegionAvail.x -= style.ItemSpacing.x * 2;
			
			//子窗口可用大小
			float fNodeTreeWidth = v2RegionAvail.x * 0.25f;
			float fPropertyWidth = v2RegionAvail.x * 0.35f;
			if (fNodeTreeWidth > 200.0f)
				fNodeTreeWidth = 200.0f;
			if (fPropertyWidth > 400.0f)
				fPropertyWidth = 400.0f;

			if (ImGui::BeginChild("NodeTree", ImVec2(fNodeTreeWidth, v2RegionAvail.y), false, unWindowFlags))
			{		
				m_pNodeTreeView->Render();
			}
			ImGui::EndChild();

			ImGui::SameLine();


			if (ImGui::BeginChild("Render", ImVec2(v2RegionAvail.x - fNodeTreeWidth - fPropertyWidth, v2RegionAvail.y), false, unWindowFlags))
			{
				ImVec2 v2RenderViewPos = ImGui::GetWindowPos();
				ImVec2 v2RenderViewSize = ImGui::GetWindowSize();

				m_vViewport[0]->SetLeftTop(Vector2(v2RenderViewPos.x, v2RenderViewPos.y));
				m_vViewport[0]->SetSize(Vector2(v2RenderViewSize.x, v2RenderViewSize.y));
			}
			ImGui::EndChild();


			ImGui::SameLine();

			if (ImGui::BeginChild("Property", ImVec2(fPropertyWidth, v2RegionAvail.y), false, unWindowFlags))
			{
				if (MNode * pNode = dynamic_cast<MNode*>(m_pNodeTreeView->GetSelectionNode()))
				{
					ImGui::Text(pNode->GetName().c_str());
				}
				m_pPropertyView->SetEditorObject(m_pNodeTreeView->GetSelectionNode());
				//MNode* pNode = m_pScene->GetRootNode()->FindFirstChildByName("Teaport");
				MNode* pNode = dynamic_cast<MNode*>(m_pNodeTreeView->GetSelectionNode());
				m_pScene->GetTransformCoord()->SetTarget3DNode(pNode);
				m_pPropertyView->Render();
			}
			ImGui::EndChild();
		}

		ImGui::End();

	}


	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

void MainEditor::OnRenderEnd()
{	

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
