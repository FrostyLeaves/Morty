#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_win32.h"


#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "imgui_impl_dx11.h"
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "imgui_impl_vulkan.h"
#include "Vulkan/MVulkanWrapper.h"
#include "Vulkan/MVulkanRenderer.h"
#include "Vulkan/MVulkanRenderTarget.h"
#endif

#include "MDirectX11RenderTarget.h"
#include "Vulkan/MVulkanRenderTarget.h"

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
#include "MForwardRenderProgram.h"

#include "Matrix.h"
#include "MTexture.h"

#include "NodeTreeView.h"
#include "PropertyView.h"
#include "MaterialView.h"
#include "ResourceView.h"

#include "MRenderStatistics.h"
#include "MInputManager.h"

#include "NotifyManager.h"

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
	, m_bShowRenderView(true)
	, m_bShowMaterial(false)
	, m_bShowResource(true)
	, m_ImguiRenderPass()
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

	
	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(GetHWND());

	m_ImguiRenderPass.m_vBackDesc.push_back(MRenderPass::MTargetDesc(true, MColor(0.0f, 0.0f, 0.0f, 1.0f)));
	m_pEngine->GetDevice()->GenerateRenderPass(&m_ImguiRenderPass, GetRenderTarget());

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	MDirectX11Device* pDevice = dynamic_cast<MDirectX11Device*>(m_pEngine->GetDevice());
	ImGui_ImplDX11_Init(pDevice->m_pD3dDevice, pDevice->m_pD3dContext);
#elif RENDER_GRAPHICS == MORTY_VULKAN
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(m_pEngine->GetDevice());
	MVulkanRenderTarget* pRenderTarget = dynamic_cast<MVulkanRenderTarget*>(GetRenderTarget());

	ImGui_ImplVulkan_InitInfo vulkanInitInfo = {};

	vulkanInitInfo.Allocator = nullptr;
	vulkanInitInfo.CheckVkResultFn = nullptr;
	vulkanInitInfo.DescriptorPool = pDevice->m_ObjectDestructor.m_VkDescriptorPool;
	vulkanInitInfo.Device = pDevice->m_VkDevice;
	vulkanInitInfo.ImageCount = pRenderTarget->m_vBufferInfo.size();
	vulkanInitInfo.Instance = pDevice->m_VkInstance;
	vulkanInitInfo.MinImageCount = pRenderTarget->m_unMinImageCount;
	vulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	vulkanInitInfo.PhysicalDevice = pDevice->m_VkPhysicalDevice;
	vulkanInitInfo.PipelineCache = VK_NULL_HANDLE;
	vulkanInitInfo.Queue = pDevice->m_VkGraphicsQueue;
	vulkanInitInfo.QueueFamily = pDevice->FindQueueGraphicsFamilies(pDevice->m_VkPhysicalDevice);

	MVulkanRenderTarget* pVulkanRenderTarget = dynamic_cast<MVulkanRenderTarget*>(GetRenderTarget());
	pVulkanRenderTarget->RegisterRenderProgram<MForwardRenderProgram>();
	MIRenderProgram* pRenderProgram = pVulkanRenderTarget->GetRenderProgram();

	
	ImGui_ImplVulkan_Init(&vulkanInitInfo, m_ImguiRenderPass.m_aVkRenderPass[0], pDevice);
	

	VkCommandBuffer buffer = pDevice->BeginCommands();
	ImGui_ImplVulkan_CreateFontsTexture(buffer);
	pDevice->EndCommands(buffer);
#endif

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

	NotifyManager::GetInstance()->RegisterNotify("Edit Material", this, NOTIFY_FUNC(this, MainEditor::Notify_Edit_Material));

	return true;
}

void MainEditor::Release()
{
	m_pEngine->GetDevice()->DestroyRenderPass(&m_ImguiRenderPass);

	m_SceneTexture.Release();

	for (IBaseView* pChild : m_vChildView)
	{
		pChild->Release();
		delete pChild;
	}

	m_vChildView.clear();

	// Cleanup
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ImGui_ImplDX11_Shutdown();
#elif RENDER_GRAPHICS == MORTY_VULKAN
	ImGui_ImplVulkan_Shutdown();
#endif

	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	MWindowsRenderView::Release();
}

void MainEditor::OnResize(const int& nWidth, const int& nHeight)
{
	MWindowsRenderView::OnResize(nWidth, nHeight);

	if (nWidth == 0 || nHeight == 0)
		return;

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
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ImGui_ImplDX11_NewFrame();
#elif RENDER_GRAPHICS == MORTY_VULKAN
	ImGui_ImplVulkan_NewFrame();
#endif

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

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#elif RENDER_GRAPHICS == MORTY_VULKAN

	if (MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(m_pEngine->GetRenderer()))
	{
		if (VkCommandBuffer vkCmmandBuffer = pVkRenderer->GetCommandBuffer())
		{
			pVkRenderer->BeginRenderPass(&m_ImguiRenderPass, GetRenderTarget());

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCmmandBuffer);

			pVkRenderer->EndRenderPass();
		}
	}
#endif
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

		SetBackColor(MColor(v4BackgroundColor.x, v4BackgroundColor.y, v4BackgroundColor.z, v4BackgroundColor.w));
	}
}

void MainEditor::Notify_Edit_Material(const MVariant& var)
{
	if (const int* pID = var.GetInt())
	{
		MResource* pResource = m_pEngine->GetResourceManager()->FindResourceByID(*pID);
		if (MMaterial* pMaterial = dynamic_cast<MMaterial*>(pResource))
		{
			m_bShowMaterial = true;
			m_pMaterialView->SetMaterial(pMaterial);
		}
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

		uint32_t unFrameIdx = m_pEngine->GetRenderer()->GetFrameIndex();
		if (void* pTexture = m_SceneTexture.GetTexture(unFrameIdx))
		{
			ImTextureID texid;
			texid.pTexture = pTexture;
			texid.nType = 0;
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

	if (!m_ImguiRenderPass.m_vBackDesc.empty())
	{
		m_ImguiRenderPass.m_vBackDesc[0].cClearColor = GetBackColor();
	}
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainEditor::ViewProcessFunction(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
		return true;

	return MWindowsRenderView::ViewProcessFunction(hwnd, message, wParam, lParam);
}
