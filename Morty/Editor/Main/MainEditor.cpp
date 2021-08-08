#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "SDL.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MRenderGlobal.h"
#include "MVulkanDevice.h"
#include <SDL_vulkan.h>
#endif

#include "MMesh.h"
#include "Matrix.h"
#include "MScene.h"
#include "MEngine.h"
#include "MObject.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MMaterial.h"
#include "MFunction.h"
#include "MTaskGraph.h"
#include "MInputManager.h"
#include "MRenderCommand.h"


#include "NodeTreeView.h"
#include "PropertyView.h"
#include "MaterialView.h"
#include "ResourceView.h"
#include "TaskGraphView.h"

#include "NotifyManager.h"

#include "ImGuiRenderable.h"

#include "MCameraComponent.h"
#include "MRenderableMeshComponent.h"

#include "MRenderSystem.h"


class MainEditorTask : public MTaskNode
{

};

MainEditor::MainEditor()
	: MRenderView()
	, m_pNodeTreeView(nullptr)
	, m_pPropertyView(nullptr)
	, m_pMaterialView(nullptr)
	, m_pResourceView(nullptr)
	, m_pTaskGraphView(nullptr)
	, m_pImGuiRenderable(nullptr)
	, m_unTriangleCount(0)
	, m_bShowMessage(true)
	, m_bShowNodeTree(true)
	, m_bShowProperty(true)
	, m_bShowRenderView(true)
	, m_bRenderToWindow(false)
	, m_bShowMaterial(false)
	, m_bShowResource(false)
	, m_bShowRenderGraph(false)
	, m_funcCloseCallback(nullptr)
	, m_pSDLWindow(nullptr)
    , m_v2DrawableSize(800.0f, 480.0f)
    , m_bWindowMinimized(false)
{
}

MainEditor::~MainEditor()
{

}

bool MainEditor::Initialize(MEngine* pEngine, const char* svWindowName)
{
	MRenderView::Initialize(pEngine);

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

	m_pImGuiRenderable = new ImGuiRenderable(pEngine);
	m_pImGuiRenderable->Initialize();

	//Setup Render
	m_SceneTexture.Initialize(pEngine);

	m_pNodeTreeView = new NodeTreeView();
	m_pPropertyView = new PropertyView();
	m_pMaterialView = new MaterialView();
	m_pResourceView = new ResourceView();
	m_pTaskGraphView = new TaskGraphView();

	m_vChildView.push_back(m_pNodeTreeView);
	m_vChildView.push_back(m_pPropertyView);
	m_vChildView.push_back(m_pMaterialView);
	m_vChildView.push_back(m_pResourceView);
	m_vChildView.push_back(m_pTaskGraphView);


	for (IBaseView* pChild : m_vChildView)
		pChild->Initialize(pEngine);

	m_pNodeTreeView->SetScene(m_SceneTexture.GetScene());

//	m_pTaskGraphView->SetRenderGraph(m_SceneTexture.GetRenderGraph());


	MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();

	MainEditorTask* pEditorTask = pMainGraph->AddNode<MainEditorTask>("Editor_Update");
	pEditorTask->SetThreadType(METhreadType::EMainThread);
	pEditorTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MainEditor::MainLoop, this));

	MTaskNode* pRenderTask = pMainGraph->AddNode<MTaskNode>("Editor_Render");
	pRenderTask->SetThreadType(METhreadType::ERenderThread);
	pRenderTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MainEditor::Render, this));

	MTaskNode* pRenderSceneTask = pMainGraph->AddNode<MTaskNode>("Scene_Render");
	pRenderSceneTask->SetThreadType(METhreadType::ERenderThread);
	pRenderSceneTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MainEditor::SceneRender, this));

	pRenderSceneTask->AppendOutput()->LinkTo(pRenderTask->AppendInput());

	return true;
}

void MainEditor::Release()
{
	MRenderView::Release();

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

	ImGui_ImplSDL2_Shutdown();

	ImGui::DestroyContext();


	SDL_DestroyWindow(m_pSDLWindow);
	m_pSDLWindow = nullptr;
}

void MainEditor::OnResize(const int& nWidth, const int& nHeight)
{
	if (nWidth == 0 || nHeight == 0)
		return;

    int w, h;
    SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);
    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;
    
    MRenderView::Resize(m_v2DrawableSize);
}

void MainEditor::Input(MInputEvent* pEvent)
{
	if (MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent))
	{
		MMouseInputEvent event(*pMouseEvent);

	}
	m_SceneTexture.GetViewport()->Input(pEvent);
}

bool MainEditor::MainLoop(MTaskNode* pNode)
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
			if (MViewport* pViewport = m_SceneTexture.GetViewport())
			{
				pViewport->Input(&e);
			}
		}
		else if (event.type == SDL_KEYUP && event.key.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::UP);
			if (MViewport* pViewport = m_SceneTexture.GetViewport())
			{
				pViewport->Input(&e);
			}
		}

		else if (event.type == SDL_MOUSEBUTTONUP && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonUp);
			if (MViewport* pViewport = m_SceneTexture.GetViewport())
			{
				pViewport->Input(&e);
			}
		}

		else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonDown);
			if (MViewport* pViewport = m_SceneTexture.GetViewport())
			{
				pViewport->Input(&e);
			}
		}

		else if (event.type == SDL_MOUSEMOTION && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			Vector2 new_pos(event.button.x, event.button.y);

			static Vector2 test_v2 = Vector2(-1, -1);
			if (test_v2.x == -1 && test_v2.y == -1)
				test_v2 = new_pos;

			MMouseInputEvent e(new_pos, new_pos - test_v2);
			test_v2 = new_pos;

			if (MViewport* pViewport = m_SceneTexture.GetViewport())
			{
				pViewport->Input(&e);
			}
		}
	}

	if (bClosed)
	{
		m_funcCloseCallback();
	}

	return !bClosed;
}

void MainEditor::InitializeSDLWindow()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(pRenderSystem->GetDevice());

    SDL_SetMainReady();
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}

    
#if defined(MORTY_WIN) || defined(MORTY_MACOS)
	// Setup window
	SDL_WindowFlags window_flags = SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_pSDLWindow = SDL_CreateWindow("Morty Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GetWidth(), GetHeight(), window_flags);
  
#elif defined(MORTY_IOS)
    m_pSDLWindow = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
#endif
    

	uint32_t unCount = 0;
	std::vector<const char*> extensions;
	SDL_Vulkan_GetInstanceExtensions(m_pSDLWindow, &unCount, nullptr);
	
	extensions.resize(unCount);

	if(SDL_Vulkan_GetInstanceExtensions(m_pSDLWindow, &unCount, extensions.data()) == 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}

	// Create Window Surface
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(m_pSDLWindow, pDevice->m_VkInstance, &surface) == 0)
	{
		printf("Error: %s\n", SDL_GetError());
		printf("Failed to create Vulkan surface.\n");
		return;
	}
    
    int w, h;
    SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);
    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;

	MRenderView::InitializeForVulkan(pDevice, surface);

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
		
			if (ImGui::MenuItem("Render to Window", "", &m_bRenderToWindow)) {}

			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Convert model"))
			{

			}

			if (ImGui::MenuItem("Load model"))
			{

			}

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


		if (!m_bRenderToWindow)
		{
			if (MTexture* pTexture = m_SceneTexture.GetTexture())
			{
				if (ImTextureID texid = pTexture)
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
		MEntity* pNode = nullptr;
		if (pNode = dynamic_cast<MEntity*>(m_pNodeTreeView->GetSelectionNode()))
		{
			ImGui::Text(pNode->GetName().c_str());
		}
		m_pPropertyView->SetEditorNode(pNode);
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
		int nCurrentFps = (int)round(GetEngine()->GetFPS() / 5) * 5;
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
		m_pTaskGraphView->Render();
	}

	ImGui::End();
}

void MainEditor::Render(MTaskNode* pNode)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pRenderCommand = pDevice->CreateRenderCommand();
	MRenderTarget* pRenderTarget = GetNextRenderTarget();
	pRenderTarget->BindPrimaryCommand(pRenderCommand);

	if (!pRenderCommand)
		return;

	pRenderCommand->RenderCommandBegin();

#if MORTY_RENDER_DATA_STATISTICS
	MRenderStatistics::GetInstance()->unTriangleCount = 0;
#endif

	Vector2 pos, size;
	if (m_bRenderToWindow)
	{
		pos = Vector2(0.0f, 0.0f);
		ImGuiIO& io = ImGui::GetIO();
		size = Vector2(io.DisplaySize.x, io.DisplaySize.y);
	}
	else
	{
		pos = Vector2(m_v2RenderViewPos.x, m_v2RenderViewPos.y);
		size = m_v2RenderViewSize;
	}

	if (MViewport* pViewport = m_SceneTexture.GetViewport())
	{
		pViewport->SetScreenPosition(pos);
	}
	if (m_SceneTexture.GetSize().x != size.x || m_SceneTexture.GetSize().y != size.y)
	{
		m_SceneTexture.SetSize(Vector2(size.x, size.y));
	}

	//m_SceneTexture.UpdateTexture(pRenderCommand);

	//m_unTriangleCount = MRenderStatistics::GetInstance()->unTriangleCount;


	
	if (m_pMaterialView && m_bShowMaterial)
	{
		if (MEntity* pEntity = m_pNodeTreeView->GetSelectionNode())
		{
			if(MRenderableMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderableMeshComponent>())
				m_pMaterialView->SetMaterial(pMeshComponent->GetMaterial());
		}
		m_pMaterialView->UpdateTexture(pRenderCommand);
	}

	ImGui_ImplSDL2_NewFrame(m_pSDLWindow);

	ImGui::NewFrame();

	if (m_bRenderToWindow)
	{
		if (MTexture* pTexture = m_SceneTexture.GetTexture())
		{
			if (ImTextureID texid = pTexture)
			{
				ImGuiIO& io = ImGui::GetIO();
				ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
				ImGui::SetNextWindowBgAlpha(0);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
				ImGui::Begin("±³¾°", NULL, ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoBringToFrontOnFocus |
					ImGuiWindowFlags_NoInputs |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoScrollbar);
				ImGui::Image(texid, ImGui::GetWindowSize());
				ImGui::End();
				ImGui::PopStyleVar(2);
			}
		}
	}

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
		pRenderCommand->BeginRenderPass(&pRenderTarget->renderPass);
		m_pImGuiRenderable->Tick(0.0f);
		m_pImGuiRenderable->Render(pRenderCommand);
		pRenderCommand->EndRenderPass();
	}


	pRenderCommand->RenderCommandEnd();

	Present(pRenderTarget);
}

void MainEditor::SceneRender(MTaskNode* pNode)
{
	m_SceneTexture.Render();
}
