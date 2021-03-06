#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "SDL.h"

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#include "MDirectX11/MDirectX11Device.h"
#include "MDirectX11/MDirectX11RenderTarget.h"
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "Vulkan/MVulkanWrapper.h"
#include "Vulkan/MVulkanRenderer.h"
#include "Vulkan/MVulkanRenderTarget.h"
#include <SDL_vulkan.h>
#endif

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
#include "MIRenderTarget.h"
#include "MTransformCoord.h"
#include "MRenderGraphTexture.h"
#include "MForwardRenderProgram.h"

#include "Matrix.h"
#include "MTexture.h"

#include "NodeTreeView.h"
#include "PropertyView.h"
#include "MaterialView.h"
#include "ResourceView.h"
#include "RenderGraphView.h"

#include "MRenderStatistics.h"
#include "MInputManager.h"

#include "NotifyManager.h"

#include "ImGuiRenderable.h"

MainEditor::MainEditor()
	: MIRenderView()
	, m_pNodeTreeView(nullptr)
	, m_pPropertyView(nullptr)
	, m_pMaterialView(nullptr)
	, m_pResourceView(nullptr)
	, m_pRenderGraphView(nullptr)
	, m_pImGuiRenderable(nullptr)
	, m_unTriangleCount(0)
	, m_bShowMessage(true)
	, m_bShowNodeTree(true)
	, m_bShowProperty(true)
	, m_bShowRenderView(true)
	, m_bShowMaterial(false)
	, m_bShowResource(false)
	, m_bShowRenderGraph(false)
	, m_funcCloseCallback(nullptr)
	, m_pSDLWindow(nullptr)
    , m_v2WindowSize(800.0f, 480.0f)
    , m_v2DrawableSize(800.0f, 480.0f)
    , m_bWindowMinimized(false)
{
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
	//Setup Window
	if (!MIRenderView::Initialize(pEngine, svWindowName))
		return false;


	//Setup ImGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.WindowPadding = ImVec2(2.0f, 2.0f);
	style.ItemSpacing.x = 2.0f;

	ImVec4 bgColor = style.Colors[ImGuiCol_WindowBg];

	io.ConfigWindowsMoveFromTitleBarOnly = true;

	InitializeSDLWindow();

	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(m_pEngine->GetDevice());
	MVulkanRenderTarget* pRenderTarget = dynamic_cast<MVulkanRenderTarget*>(GetRenderTarget());
	pRenderTarget->RegisterRenderProgram<MForwardRenderProgram>();

	m_pImGuiRenderable = new ImGuiRenderable(pEngine);
	m_pImGuiRenderable->Initialize();

	//Setup Render
	m_SceneTexture.Initialize(pEngine);
	//m_SceneTexture.SetBackColor(MColor(bgColor.x + 0.3f, bgColor.y + 0.3f, bgColor.z + 0.3f, bgColor.w));

	m_pEngine->SetScene(m_SceneTexture.GetScene());

	m_pNodeTreeView = new NodeTreeView();
	m_pPropertyView = new PropertyView();
	m_pMaterialView = new MaterialView();
	m_pResourceView = new ResourceView();
	m_pRenderGraphView = new RenderGraphView();

	m_vChildView.push_back(m_pNodeTreeView);
	m_vChildView.push_back(m_pPropertyView);
	m_vChildView.push_back(m_pMaterialView);
	m_vChildView.push_back(m_pResourceView);
	m_vChildView.push_back(m_pRenderGraphView);


	for (IBaseView* pChild : m_vChildView)
		pChild->Initialize(pEngine);

	NotifyManager::GetInstance()->RegisterNotify("Edit Material", this, NOTIFY_FUNC(this, MainEditor::Notify_Edit_Material));


	m_pRenderGraphView->SetRenderGraph(m_SceneTexture.GetRenderGraph());

	return true;
}

void MainEditor::Release()
{
	if (m_funcCloseCallback)
		m_funcCloseCallback();

	if (m_pImGuiRenderable)
	{
		m_pImGuiRenderable->Release();
		delete m_pImGuiRenderable;
		m_pImGuiRenderable = nullptr;
	}

	m_SceneTexture.Release();

	for (IBaseView* pChild : m_vChildView)
	{
		pChild->Release();
		delete pChild;
	}

	m_vChildView.clear();


	if (MIRenderTarget* pRenderTarget = GetRenderTarget())
	{
		pRenderTarget->DeleteLater();
		SetRenderTarget(NULL);
	}

	// Cleanup
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	ImGui_ImplDX11_Shutdown();
#elif RENDER_GRAPHICS == MORTY_VULKAN
#endif

	ImGui_ImplSDL2_Shutdown();

	ImGui::DestroyContext();

}

void MainEditor::OnResize(const int& nWidth, const int& nHeight)
{
	if (nWidth == 0 || nHeight == 0)
		return;

	m_v2WindowSize.x = nWidth;
	m_v2WindowSize.y = nHeight;
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);
    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;
    
    GetRenderTarget()->Resize(m_v2DrawableSize);
}

void MainEditor::Input(MInputEvent* pEvent)
{
	if (MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent))
	{
		MMouseInputEvent event(*pMouseEvent);

	}
	m_SceneTexture.GetViewport()->Input(pEvent);
}

bool MainEditor::MainLoop(const float& fDelta)
{
	SDL_Event event;
	bool bClosed = false;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			bClosed = true;

        if(event.type == SDL_WINDOWEVENT && event.window.windowID == SDL_GetWindowID(m_pSDLWindow))
        {
            if(event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                OnResize(event.window.data1, event.window.data2);
            }
            
            else if(event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                m_bWindowMinimized = true;
            else if(event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
                m_bWindowMinimized = false;
            else if(event.window.event == SDL_WINDOWEVENT_RESTORED)
                m_bWindowMinimized = false;
            
            else if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                bClosed = true;
        }
		else if (event.type == SDL_KEYDOWN && event.key.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::DOWN);
			m_SceneTexture.GetViewport()->Input(&e);
		}
		else if (event.type == SDL_KEYUP && event.key.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::UP);
			m_SceneTexture.GetViewport()->Input(&e);
		}

		else if (event.type == SDL_MOUSEBUTTONUP && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonUp);
			m_SceneTexture.GetViewport()->Input(&e);
		}

		else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonDown);
			m_SceneTexture.GetViewport()->Input(&e);
		}

		else if (event.type == SDL_MOUSEMOTION && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			Vector2 new_pos(event.button.x, event.button.y);

			static Vector2 test_v2 = Vector2(-1, -1);
			if (test_v2.x == -1 && test_v2.y == -1)
				test_v2 = new_pos;

			MMouseInputEvent e(new_pos, new_pos - test_v2);
			test_v2 = new_pos;

			m_SceneTexture.GetViewport()->Input(&e);
		}
	}

	if (bClosed)
	{
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = nullptr;
	}

	return !bClosed;
}

void MainEditor::SetRenderTarget(MIRenderTarget* pRenderTarget)
{
	MIRenderView::SetRenderTarget(pRenderTarget);

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

void MainEditor::InitializeSDLWindow()
{
    
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(m_pEngine->GetDevice());

    SDL_SetMainReady();
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}

    
#if defined(MORTY_WIN) || defined(MORTY_MACOS)
	// Setup window
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_pSDLWindow = SDL_CreateWindow("Morty Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_v2WindowSize.x, m_v2WindowSize.y, window_flags);
  
#elif defined(MORTY_IOS)
    m_pSDLWindow = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
#endif
    

	// Create Window Surface
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(m_pSDLWindow, pDevice->m_VkInstance, &surface) == 0)
	{
		printf("Failed to create Vulkan surface.\n");
		return;
	}
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);
    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;

	MVulkanRenderTarget::CreateForSurface(m_pEngine, this, surface);


	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForVulkan(m_pSDLWindow);
     
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
			if (ImGui::MenuItem("RenderGraph", "", &m_bShowRenderGraph)) {}
			
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

		if (MRenderGraph* pRenderGraph = m_SceneTexture.GetRenderGraph())
		{
			if (MRenderGraphTexture* pRenderGraphTexture = m_pRenderGraphView->GetSelectedOutputTexture())
			{
				if (ImTextureID texid = pRenderGraphTexture->GetRenderTexture())
				{
					ImGui::Image(texid, v2RenderViewSize);
				}
			}
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

void MainEditor::ShowRenderGraphView()
{
	if (!m_bShowRenderGraph)
		return;

	if (ImGui::Begin("RenderGraph", &m_bShowRenderGraph))
	{
		m_pRenderGraphView->Render();
	}

	ImGui::End();
}

void MainEditor::Render()
{
	MRenderCommand* pRenderCommand = nullptr;
	MIRenderer* pRenderer = m_pEngine->GetRenderer();

#if RENDER_GRAPHICS == MORTY_VULKAN

	MVulkanRenderer* pVkRenderer = dynamic_cast<MVulkanRenderer*>(m_pEngine->GetRenderer());
	if (!pVkRenderer)
		return;
	MVulkanRenderTarget* pRenderTarget = dynamic_cast<MVulkanRenderTarget*>(GetRenderTarget());
	if (!pRenderTarget)
		return;

	pRenderCommand = pRenderTarget->GetPrimaryCommand();
	
#endif;

	if (!pRenderCommand)
		return;


	GetRenderTarget()->WaitImageReady();
	pRenderer->RenderCommandBegin(pRenderCommand);

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

		m_SceneTexture.UpdateTexture(pRenderCommand);
	}
	m_unTriangleCount = MRenderStatistics::GetInstance()->unTriangleCount;


	
	if (m_pMaterialView && m_bShowMaterial)
	{
		if (MIMeshInstance* pMeshIns = m_pNodeTreeView->GetSelectionNode()->DynamicCast<MIMeshInstance>())
		{
			m_pMaterialView->SetMaterial(pMeshIns->GetMaterial());
		}
		m_pMaterialView->UpdateTexture(pRenderCommand);
	}

	ImGui_ImplSDL2_NewFrame(m_pSDLWindow);

	ImGui::NewFrame();

	ShowMenu();
	ShowMaterial();
	ShowRenderView();
	ShowNodeTree();
	ShowProperty();
	ShowMessage();
	ShowResource();
	ShowRenderGraphView();

	// Rendering
	ImGui::Render();



	if (m_pImGuiRenderable)
	{
		pVkRenderer->BeginRenderPass(pRenderCommand, &pRenderTarget->m_RenderPass, pRenderTarget->GetFrameBufferIndex());
		m_pImGuiRenderable->Tick(0.0f);
		m_pImGuiRenderable->Render(m_pEngine->GetRenderer(), pRenderCommand);
		pVkRenderer->EndRenderPass(pRenderCommand);
	}


	pVkRenderer->RenderCommandEnd(pRenderCommand);


	pVkRenderer->SubmitRenderCommand(pRenderCommand, pRenderTarget);
	GetRenderTarget()->Present(pRenderCommand);
}
