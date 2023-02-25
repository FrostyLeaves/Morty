#include "MainEditor.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "ImGuiFileDialog.h"

#include "SDL.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/MRenderGlobal.h"
#include "Render/Vulkan/MVulkanDevice.h"
#include <SDL_vulkan.h>
#endif

#include "Render/MMesh.h"
#include "Math/Matrix.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Material/MMaterial.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskGraph.h"
#include "Input/MInputEvent.h"
#include "Render/MRenderCommand.h"


#include "Widget/NodeTreeView.h"
#include "Widget/PropertyView.h"
#include "Widget/MaterialView.h"
#include "Widget/ResourceView.h"
#include "Widget/TaskGraphView.h"
#include "Widget/ModelConvertView.h"
#include "Widget/MessageView.h"

#include "Utility/NotifyManager.h"

#include "Render/ImGuiRenderable.h"

#include "Component/MCameraComponent.h"
#include "Component/MRenderableMeshComponent.h"

#include "System/MInputSystem.h"
#include "System/MRenderSystem.h"

#include "RenderProgram/MDeferredRenderProgram.h"


MString MainEditor::m_sRenderProgramName = MDeferredRenderProgram::GetClassTypeName();

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
	, m_pModelConvertView(nullptr)
	, m_pMessageView(nullptr)
	, m_pImGuiRenderable(nullptr)
	, m_unTriangleCount(0)
	, m_bRenderToWindow(true)
	, m_bShowRenderView(false)
	, m_bShowDebugView(false)
	, m_funcCloseCallback(nullptr)
	, m_pSDLWindow(nullptr)
    , m_v2DrawableSize(80.0f, 48.0f)
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
	m_SceneTexture.Initialize(pEngine, m_sRenderProgramName, GetImageCount());

	m_pNodeTreeView = new NodeTreeView();
	m_pPropertyView = new PropertyView();
	m_pMaterialView = new MaterialView();
	m_pResourceView = new ResourceView();
	m_pTaskGraphView = new TaskGraphView();
	m_pModelConvertView = new ModelConvertView();
	m_pMessageView = new MessageView();

	m_vChildView.push_back(m_pNodeTreeView);
	m_vChildView.push_back(m_pPropertyView);
	m_vChildView.push_back(m_pMaterialView);
	m_vChildView.push_back(m_pResourceView);
	m_vChildView.push_back(m_pTaskGraphView);
	m_vChildView.push_back(m_pModelConvertView);
	m_vChildView.push_back(m_pMessageView);

	for (IBaseView* pChild : m_vChildView)
		pChild->Initialize(pEngine);

	m_pNodeTreeView->SetScene(m_SceneTexture.GetScene());

	m_pNodeTreeView->m_bVisiable = true;
	m_pPropertyView->m_bVisiable = true;


	MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();

	MainEditorTask* pEditorTask = pMainGraph->AddNode<MainEditorTask>("Editor_Update");
	pEditorTask->SetThreadType(METhreadType::ECurrentThread);
	pEditorTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MainEditor::MainLoop, this));

	MTaskNode* pRenderTask = pMainGraph->AddNode<MTaskNode>("Editor_Render");
	pRenderTask->SetThreadType(METhreadType::ERenderThread);
	pRenderTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MainEditor::Render, this));
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

	if (w == 0 || h == 0)
		return;

    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;

	m_bWindowResized = true;
}

void MainEditor::Input(MInputEvent* pEvent)
{
	if (MInputSystem* pInputSystem = GetEngine()->FindSystem<MInputSystem>())
	{
		pInputSystem->Input(pEvent);
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
			Input(&e);
		}
		else if (event.type == SDL_KEYUP && event.key.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::UP);
			Input(&e);
		}

		else if (event.type == SDL_MOUSEBUTTONUP && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonUp);
			Input(&e);
		}

		else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MMouseInputEvent::MEMouseDownButton type;
			if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
			if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
			if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

			MMouseInputEvent e(type, MMouseInputEvent::ButtonDown);
			Input(&e);
		}

		else if (event.type == SDL_MOUSEMOTION && event.button.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			Vector2 new_pos(event.button.x, event.button.y);

			static Vector2 test_v2 = Vector2(-1, -1);
			if (test_v2.x == -1 && test_v2.y == -1)
				test_v2 = new_pos;

			MMouseInputEvent e(new_pos, new_pos - test_v2);
			test_v2 = new_pos;

			Input(&e);
		}
	}

	if (bClosed)
	{
		m_funcCloseCallback();
	}
	else
	{
		if (MScene* pScene = m_SceneTexture.GetScene())
		{
			pScene->Tick(GetEngine()->getTickDelta());
		}
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
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", ""))
			{
				ImGuiFileDialog::Instance()->OpenModal("OpenFile", "Open", "entity\0\0", ".");
			}

			if (ImGui::MenuItem("Save", ""))
			{

			}

			if (ImGui::MenuItem("Save as", ""))
			{
				ImGuiFileDialog::Instance()->OpenModal("Save As", "Open", "entity\0\0", "new");
			}


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Render", "", &m_bShowRenderView)) {}
			if (ImGui::MenuItem("DebugTexture", "", &m_bShowDebugView)) {}
			if (ImGui::MenuItem("NodeTree", "", &m_pNodeTreeView->m_bVisiable)) {}
			if (ImGui::MenuItem("Property", "", &m_pPropertyView->m_bVisiable)) {}
			if (ImGui::MenuItem("Material", "", &m_pMaterialView->m_bVisiable)) {}
			if (ImGui::MenuItem("Resource", "", &m_pResourceView->m_bVisiable)) {}
			if (ImGui::MenuItem("Message", "", &m_pMessageView->m_bVisiable)) {}
			if (ImGui::MenuItem("ModelConvert", "", &m_pModelConvertView->m_bVisiable)) {}
			if (ImGui::MenuItem("RenderGraph", "", &m_pTaskGraphView->m_bVisiable)) {}
		
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

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tool"))
		{
			if (ImGui::MenuItem("Snip shot"))
			{
				m_SceneTexture.Snapshot("./test.png");
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void MainEditor::ShowRenderView(const size_t& nImageCount)
{
	if (!m_bShowRenderView)
		return;

	if (ImGui::Begin("Render", &m_bShowRenderView, ImGuiWindowFlags_NoCollapse))
	{
		if (!m_bRenderToWindow)
		{
			if (std::shared_ptr<MTexture> pTexture = m_SceneTexture.GetTexture(nImageCount))
			{
				ImTextureID texid = { pTexture, 0 };

				Vector4 v4Rect = GetWidgetSize();

				m_v2RenderViewPos.x = v4Rect.m[0];
				m_v2RenderViewPos.y = v4Rect.m[1];

				m_v2RenderViewSize.x = v4Rect.m[2];
				m_v2RenderViewSize.y = v4Rect.m[3];

				ImGui::Image(texid, ImVec2(m_v2RenderViewSize.x, m_v2RenderViewSize.y));
			}
			
		}
	}
	ImGui::End();
}

void MainEditor::ShowShadowMapView(const size_t& nImageCount)
{
	if (!m_bShowDebugView)
		return;

	if (ImGui::Begin("DebugView", &m_bShowDebugView))
	{
		std::vector<std::shared_ptr<MTexture>> vTexture = m_SceneTexture.GetAllOutputTexture(nImageCount);
		if(!vTexture.empty())
		{
			size_t nImageSize = 0;
			for (std::shared_ptr<MTexture> pTexture : vTexture)
			{
				if (pTexture)
				{
					nImageSize += pTexture->GetImageLayerNum();
				}
			}
			
			// n * n
			size_t nRowCount = std::ceil(std::sqrt(nImageSize));

			Vector4 v4Rect = GetWidgetSize();

			Vector2 v2Size = Vector2((v4Rect.z) / nRowCount, (v4Rect.w) / nRowCount);

			ImGui::Columns(nRowCount);
			for (size_t nTexIdx = 0; nTexIdx < vTexture.size(); ++nTexIdx)
			{
				for (size_t nLayerIdx = 0; nLayerIdx < vTexture[nTexIdx]->GetImageLayerNum(); ++nLayerIdx)
				{
					ImGui::Image({ vTexture[nTexIdx], nLayerIdx }, ImVec2(v2Size.x, v2Size.y));

					ImGui::NextColumn();
				}
			}
			ImGui::Columns(1);
		
		}
	}

	ImGui::End();
}

void MainEditor::ShowNodeTree()
{
	if (!m_pNodeTreeView->m_bVisiable)
		return;

	if (ImGui::Begin("Scene", &m_pNodeTreeView->m_bVisiable))
	{
		m_pNodeTreeView->Render();
	}
	ImGui::End();
}

void MainEditor::ShowProperty()
{
	if (!m_pPropertyView->m_bVisiable)
		return;

	if (ImGui::Begin("Property", &m_pPropertyView->m_bVisiable))
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

void MainEditor::ShowMaterial(const size_t& nImageCount)
{
	if (!m_pMaterialView->m_bVisiable)
		return;

	if (ImGui::Begin("Material", &m_pMaterialView->m_bVisiable))
	{
		m_pMaterialView->Render();
	}

	ImGui::End();
}

void MainEditor::ShowMessage()
{
	if (!m_pMessageView->m_bVisiable)
		return;

	m_pMessageView->Render();
}

void MainEditor::ShowResource()
{
	if (!m_pResourceView->m_bVisiable)
		return;

	if (ImGui::Begin("Resource", &m_pResourceView->m_bVisiable))
	{
		m_pResourceView->Render();
	}

	ImGui::End();
}

void MainEditor::ShowModelConvert()
{
	if (!m_pModelConvertView->m_bVisiable)
		return;

	if (ImGui::Begin("ModelConvert", &m_pModelConvertView->m_bVisiable))
	{
		m_pModelConvertView->Render();
	}

	ImGui::End();
}

void MainEditor::ShowRenderGraphView()
{
	if (!m_pTaskGraphView->m_bVisiable)
		return;

	if (ImGui::Begin("RenderGraph", &m_pTaskGraphView->m_bVisiable))
	{
		m_pTaskGraphView->Render();
	}

	ImGui::End();
}

void MainEditor::ShowDialog()
{
	if (ImGuiFileDialog::Instance()->Display("OpenFile"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();

			int a = 0;
			++a;
		}
		ImGuiFileDialog::Instance()->Close();
	}


	if (ImGuiFileDialog::Instance()->Display("Save As"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string strFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string strCurrentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();


			int a = 0;
			++a;
		}
		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("Convert Model"))
	{

	}
}

Vector4 MainEditor::GetWidgetSize()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec2 v2RenderViewPos = ImGui::GetWindowPos();
	ImVec2 v2RenderViewSize = ImGui::GetWindowSize();

	v2RenderViewPos.x += style.WindowPadding.x;
	v2RenderViewPos.y += ImGui::GetItemRectSize().y;

	v2RenderViewSize.x -= style.WindowPadding.x * 2.0f;
	v2RenderViewSize.y -= (style.WindowPadding.y * 2.0f + ImGui::GetItemRectSize().y * 2.0f);

	return Vector4(v2RenderViewPos.x, v2RenderViewPos.y, v2RenderViewSize.x, v2RenderViewSize.y);
}

void MainEditor::Render(MTaskNode* pNode)
{
	if (GetMinimized())
		return;

	if (m_bWindowResized)
	{
		MRenderView::Resize(m_v2DrawableSize);
		m_bWindowResized = false;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pDevice = pRenderSystem->GetDevice();
	MRenderTarget* pRenderTarget = GetNextRenderTarget();
	if (!pRenderTarget)
		return;

	MIRenderCommand* pRenderCommand = pDevice->CreateRenderCommand("MainEditor RenderCommand");
	if (!pRenderCommand)
		return;

	pRenderTarget->BindPrimaryCommand(pRenderCommand);

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

	ImGui_ImplSDL2_NewFrame(m_pSDLWindow);

	ImGui::NewFrame();


	std::vector<MTexture*> vRenderTextures;
	if (m_pMaterialView && m_pMaterialView->m_bVisiable)
	{
		if (MEntity* pEntity = m_pNodeTreeView->GetSelectionNode())
		{
			if (MRenderableMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderableMeshComponent>())
				m_pMaterialView->SetMaterial(pMeshComponent->GetMaterial());
		}

		SceneTexture& sceneTexture = m_pMaterialView->GetSceneTexture();

		if (pRenderTarget->unImageIndex == 0)
		{
			sceneTexture.UpdateTexture(0, pRenderCommand);
			if (std::shared_ptr<MTexture> pTexture = sceneTexture.GetTexture(pRenderTarget->unImageIndex))
			{
				vRenderTextures.push_back(pTexture.get());
			}
		}

	}


	//Update Scene
	m_SceneTexture.UpdateTexture(pRenderTarget->unImageIndex, pRenderCommand);
	if (std::shared_ptr<MTexture> pRenderTexture = m_SceneTexture.GetTexture(pRenderTarget->unImageIndex))
	{
		vRenderTextures.push_back(pRenderTexture.get());

		pRenderCommand->AddRenderToTextureBarrier(vRenderTextures);

		if (m_bRenderToWindow)
		{
			if (pRenderTexture)
			{
				
				ImGuiIO& io = ImGui::GetIO();
				ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
				ImGui::SetNextWindowBgAlpha(0);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
				ImGui::Begin("����", NULL, ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoBringToFrontOnFocus |
					ImGuiWindowFlags_NoInputs |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoScrollbar);
				ImGui::Image({ pRenderTexture, 0 }, ImGui::GetWindowSize());
				ImGui::End();
				ImGui::PopStyleVar(2);
				
			}
		}
	}

	ShowMenu();
	ShowMaterial(pRenderTarget->unImageIndex);
	ShowRenderView(pRenderTarget->unImageIndex);
	ShowShadowMapView(pRenderTarget->unImageIndex);
	ShowNodeTree();
	ShowProperty();
	ShowMessage();
	ShowResource();
	ShowRenderGraphView();
	ShowModelConvert();
	
	ShowDialog();

	// Rendering
	ImGui::Render();



	if (m_pImGuiRenderable)
	{
		m_pImGuiRenderable->Tick(0.0f);
		m_pImGuiRenderable->WaitTextureReady(pRenderCommand);
		pRenderCommand->BeginRenderPass(&pRenderTarget->renderPass);
		m_pImGuiRenderable->Render(pRenderCommand);
		pRenderCommand->EndRenderPass();
	}
	
	pRenderCommand->RenderCommandEnd();

	m_pMessageView->SetDrawCallCount(pRenderCommand->GetDrawCallCount());

	Present(pRenderTarget);
	
}

void MainEditor::SceneRender(MTaskNode* pNode)
{
}
