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
#include "Material/MMaterialResource.h"
#include "MMaterial.h"
#include "Model/MIMeshInstance.h"
#include "MMesh.h"
#include "MScene.h"
#include "MCamera.h"
#include "MTransformCoord.h"
#include "MIRenderTarget.h"
#include "MTextureRenderTarget.h"

#include "Matrix.h"
#include "MTexture.h"

#include "NodeTreeView.h"
#include "PropertyView.h"
#include "MaterialView.h"
#include "ResourceView.h"

#include "MRenderStatistics.h"
#include "MInputManager.h"

MainEditor::MainEditor()
	: MWindowsRenderView()
	, m_pNodeTreeView(nullptr)
	, m_pPropertyView(nullptr)
	, m_pMaterialView(nullptr)
	, m_pResourceView(nullptr)
	, m_unTriangleCount(0)
	, m_bShowMessage(true)
	, m_bShowNodeTree(true)
	, m_bShowProperty(true)
	, m_bShowRenderView(false)
	, m_bShowMaterial(false)
	, m_bShowResource(true)
{
	m_nWidth = 800.0f;
	m_nHeight = 480.0f;
}

MainEditor::~MainEditor()
{

}

void MainEditor::SetEditorNode(MNode* pNode)
{
	m_SceneTexture.GetScene()->SetRootNode(pNode);
	m_pNodeTreeView->SetRootNode(pNode);
}

bool MainEditor::Initialize(MEngine* pEngine, const char* svWindowName)
{
	//Setup ImGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.WindowPadding = ImVec2(2.0f, 2.0f);
	style.ItemSpacing.x = 2.0f;

	ImVec4 bgColor = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_ChildWindowBg] = bgColor;

	io.ConfigWindowsMoveFromTitleBarOnly = true;

	//Setup Window
	if (!MWindowsRenderView::Initialize(pEngine, svWindowName))
		return false;



	MDirectX11Device* pDevice = dynamic_cast<MDirectX11Device*>(m_pEngine->GetDevice());
	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(GetHWND());
	ImGui_ImplDX11_Init(pDevice->m_pD3dDevice, pDevice->m_pD3dContext);

	//Setup Render
	m_SceneTexture.Initialize(pEngine);
	m_pEngine->SetScene(m_SceneTexture.GetScene());

	m_pNodeTreeView = new NodeTreeView();
	m_pPropertyView = new PropertyView();
	m_pMaterialView = new MaterialView();
	m_pResourceView = new ResourceView();

	m_vChildView.push_back(m_pNodeTreeView);
	m_vChildView.push_back(m_pPropertyView);
	m_vChildView.push_back(m_pMaterialView);
	m_vChildView.push_back(m_pResourceView);


	for (IBaseView* pChild : m_vChildView)
		pChild->Initialize(pEngine);
}

void MainEditor::Release()
{
	m_SceneTexture.Release();

	for (IBaseView* pChild : m_vChildView)
	{
		pChild->Release();
		delete pChild;
	}

	m_vChildView.clear();

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

void MainEditor::Input(MInputEvent* pEvent)
{
	if (MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent))
	{
		MMouseInputEvent event(*pMouseEvent);

	}
	m_SceneTexture.GetViewport()->Input(pEvent);
}

void MainEditor::OnRenderEnd()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	ShowMenu();
	ShowMaterial();
	ShowRenderView();
	ShowNodeTree();
	ShowProperty();
	ShowMessage();
	ShowResource();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void MainEditor::SetRenderTarget(MIRenderTarget* pRenderTarget)
{
	MWindowsRenderView::SetRenderTarget(pRenderTarget);

	if (pRenderTarget)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 v4BackgroundColor = style.Colors[ImGuiCol_WindowBg];

		v4BackgroundColor.x = (v4BackgroundColor.x + 0.5f) * 0.5f;
		v4BackgroundColor.y = (v4BackgroundColor.y + 0.5f) * 0.5f;
		v4BackgroundColor.z = (v4BackgroundColor.z + 0.5f) * 0.5f;
		GetRenderTarget()->m_backgroundColor = MColor(v4BackgroundColor.x, v4BackgroundColor.y, v4BackgroundColor.z, v4BackgroundColor.w);
	}
}

void MainEditor::ShowMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Render", "", &m_bShowRenderView)) {}
			if (ImGui::MenuItem("NodeTree", "", &m_bShowNodeTree)) {}
			if (ImGui::MenuItem("Property", "", &m_bShowProperty)) {}
			if (ImGui::MenuItem("Material", "", &m_bShowMaterial)) {}
			if (ImGui::MenuItem("Resource", "", &m_bShowResource)) {}
			if (ImGui::MenuItem("Message", "", &m_bShowMessage)) {}
			
			ImGui::EndMenu();
		}
		
		ImGui::EndMainMenuBar();
	}
}

void MainEditor::ShowRenderView()
{
	if (!m_bShowRenderView)
		return;

	if (ImGui::Begin("Render", &m_bShowRenderView, ImGuiWindowFlags_NoCollapse))
	{
		ImGuiStyle& style = ImGui::GetStyle();

		ImVec2 v2RenderViewPos = ImGui::GetWindowPos();
		ImVec2 v2RenderViewSize = ImGui::GetWindowSize();

		v2RenderViewPos.x += style.WindowPadding.x;
		v2RenderViewPos.y += ImGui::GetItemRectSize().y;

		v2RenderViewSize.x -= style.WindowPadding.x * 2.0f;
		v2RenderViewSize.y -= (style.WindowPadding.y * 2.0f + ImGui::GetItemRectSize().y * 2.0f);

		m_v2RenderViewPos.x = v2RenderViewPos.x;
		m_v2RenderViewPos.y = v2RenderViewPos.y;

		m_v2RenderViewSize.x = v2RenderViewSize.x;
		m_v2RenderViewSize.y = v2RenderViewSize.y;

		if (ImTextureID texid = m_SceneTexture.GetTexture())
		{
			ImGui::Image(texid, v2RenderViewSize);
		}
	}
	ImGui::End();
}

void MainEditor::ShowNodeTree()
{
	if (!m_bShowNodeTree)
		return;

	if (ImGui::Begin("Scene", &m_bShowNodeTree))
	{
		m_pNodeTreeView->Render();
	}
	ImGui::End();
}

void MainEditor::ShowProperty()
{
	if (!m_bShowProperty)
		return;

	if (ImGui::Begin("Property", &m_bShowProperty))
	{
		MNode* pNode = nullptr;
		if (pNode = dynamic_cast<MNode*>(m_pNodeTreeView->GetSelectionNode()))
		{
			ImGui::Text(pNode->GetName().c_str());
		}
		m_pPropertyView->SetEditorObject(pNode);
		m_SceneTexture.GetScene()->GetTransformCoord()->SetTarget3DNode(pNode);
		m_pPropertyView->Render();
	}
	ImGui::End();
}

void MainEditor::ShowMaterial()
{
	if (!m_bShowMaterial)
		return;

	if (ImGui::Begin("Material", &m_bShowMaterial))
	{
		m_pMaterialView->Render();
	}

	ImGui::End();
}

void MainEditor::ShowMessage()
{
	if (!m_bShowMessage)
		return;

	const float DISTANCE = 10.0f;
	static int corner = -1;
	if (corner != -1)
	{
		ImGuiIO& io = ImGui::GetIO();

		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (ImGui::Begin("Message", &m_bShowMessage, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		int nCurrentFps = (int)round(m_pEngine->GetInstantFPS() / 5) * 5;
		ImGui::Text("FPS: %d", nCurrentFps);
#if MORTY_RENDER_DATA_STATISTICS
		ImGui::Text("Triangle Count: %d", m_unTriangleCount);
#endif

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (ImGui::MenuItem("Close")) m_bShowMessage = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void MainEditor::ShowResource()
{
	if (!m_bShowResource)
		return;

	if (ImGui::Begin("Resource", &m_bShowResource))
	{
		m_pResourceView->Render();
	}

	ImGui::End();
}

void MainEditor::OnRenderBegin()
{	
#if MORTY_RENDER_DATA_STATISTICS
	MRenderStatistics::GetInstance()->unTriangleCount = 0;
#endif
	{
		MViewport* pViewport = m_SceneTexture.GetViewport();
		pViewport->SetScreenPosition(Vector2(m_v2RenderViewPos.x, m_v2RenderViewPos.y));

		if (m_SceneTexture.GetSize().x != m_v2RenderViewSize.x || m_SceneTexture.GetSize().y != m_v2RenderViewSize.y)
		{
			m_SceneTexture.SetSize(Vector2(m_v2RenderViewSize.x, m_v2RenderViewSize.y));
		}

		m_SceneTexture.UpdateTexture();
	}
	m_unTriangleCount = MRenderStatistics::GetInstance()->unTriangleCount;



	if (m_pMaterialView && m_bShowMaterial)
	{
		if (MIMeshInstance* pMeshIns = m_pNodeTreeView->GetSelectionNode()->DynamicCast<MIMeshInstance>())
		{
			m_pMaterialView->SetMaterial(pMeshIns->GetMaterial());
		}
		m_pMaterialView->UpdateMaterialTexture();
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
