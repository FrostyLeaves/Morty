#include "SDLRenderView.h"

#include "Utility/MGlobal.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "SDL.h"
#include "imgui_impl_sdl.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/MRenderGlobal.h"
#include "Render/Vulkan/MVulkanDevice.h"
#include <SDL_vulkan.h>
#endif


#include "Render/MMesh.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Input/MInputEvent.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskGraph.h"
#include "Render/MRenderCommand.h"

#include "Render/ImGuiRenderable.h"
#include "Component/MRenderMeshComponent.h"
#include "Render/ImGui/imnodes.h"

#include "System/MInputSystem.h"
#include "System/MRenderSystem.h"

#include "Utility/RenderMessageManager.h"

void SDLRenderView::Initialize(MEngine* pEngine)
{
	MRenderView::Initialize(pEngine);

	//Setup ImGui
	ImGuiContext* imGuiContext = ImGui::CreateContext();
	ImGuizmo::SetImGuiContext(imGuiContext);
	ImNodes::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.WindowPadding = ImVec2(2.0f, 2.0f);
	style.ItemSpacing.x = 2.0f;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	m_pImGuiRender = new ImGuiRenderable(pEngine);
	m_pImGuiRender->Initialize();

	MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();
	MTaskNode* pEditorTask = pMainGraph->AddNode<MTaskNode>("Window_Update");
	pEditorTask->SetThreadType(METhreadType::ECurrentThread);
	pEditorTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(SDLRenderView::MainLoop, this));

	m_pRenderTask = pMainGraph->AddNode<MTaskNode>("Window_Render");
	m_pRenderTask->SetThreadType(METhreadType::ERenderThread);
	m_pRenderTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(SDLRenderView::Render, this));

}

void SDLRenderView::Release()
{
	if (m_pImGuiRender)
	{
		m_pImGuiRender->Release();
		delete m_pImGuiRender;
		m_pImGuiRender = nullptr;
	}

	ImNodes::DestroyContext();
	ImGui::DestroyContext();

	UnbindSDLWindow();

	MRenderView::Release();
}

void SDLRenderView::Resize(const int& nWidth, const int& nHeight)
{
	if (nWidth == 0 || nHeight == 0)
	{
		return;
	}

    int w, h;
    SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);

	if (w == 0 || h == 0)
		return;

    m_v2DrawableSize.x = w;
    m_v2DrawableSize.y = h;
	m_bWindowResized = true;

	if (m_bWindowResized) //TODO android surface only active in current call stack
	{
		MRenderView::Resize(m_v2DrawableSize);
		m_bWindowResized = false;
	}


	for (auto content : m_vContent)
	{
		content->OnResize(m_v2DrawableSize);
	}
}

void SDLRenderView::AppendContent(RenderViewContent* pContent)
{
	if (auto pTaskNode = pContent->GetRenderTask())
	{
		pTaskNode->ConnectTo(GetEditorRenderTask());
	}

	m_vContent.push_back(pContent);
}

void SDLRenderView::Input(MInputEvent* pEvent)
{
	if (MInputSystem* pInputSystem = GetEngine()->FindSystem<MInputSystem>())
	{
		pInputSystem->Input(pEvent);
	}

	for (auto content : m_vContent)
	{
		content->OnInput(pEvent);
	}
}

bool SDLRenderView::MainLoop(MTaskNode* pNode)
{
	MORTY_UNUSED(pNode);

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
				Resize(event.window.data1, event.window.data2);
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
		else if (event.type == SDL_TEXTEDITING && event.key.windowID == SDL_GetWindowID(m_pSDLWindow))
		{
			MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::DOWN);
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

			static Vector2 prev_pos = Vector2(-1, -1);
			if (prev_pos.x == -1 && prev_pos.y == -1)
			{
				prev_pos = new_pos;
			}

			MMouseInputEvent e(new_pos, new_pos - prev_pos);
			prev_pos = new_pos;

			Input(&e);
		}
	}

	for (auto content : m_vContent)
	{
		content->OnTick(GetEngine()->getTickDelta());
	}


	m_bWindowClosed = bClosed;
	return !bClosed;
}

void SDLRenderView::BindSDLWindow()
{
	if (m_pSDLWindow)
	{
		UnbindSDLWindow();
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MVulkanDevice* pDevice = dynamic_cast<MVulkanDevice*>(pRenderSystem->GetDevice());

	SDL_SetMainReady();

		// Setup SDL
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		GetEngine()->GetLogger()->Error("SDL_Init failed. {}", SDL_GetError());
		return;
	}


#if defined(MORTY_WIN) || defined(MORTY_MACOS)
	// Setup window
	SDL_WindowFlags window_flags = SDL_WindowFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	m_pSDLWindow = SDL_CreateWindow("Morty Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GetWidth(), GetHeight(), window_flags);

#elif defined(MORTY_IOS) || defined(MORTY_ANDROID)
	m_pSDLWindow = SDL_CreateWindow(NULL, 0, 0, 320, 480, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI);
#endif

	if (nullptr == m_pSDLWindow)
	{
		GetEngine()->GetLogger()->Error("SDL_CreateWindow failed. {}", SDL_GetError());
}


	uint32_t unCount = 0;
	std::vector<const char*> extensions;
	SDL_Vulkan_GetInstanceExtensions(m_pSDLWindow, &unCount, nullptr);

	extensions.resize(unCount);

	if (SDL_Vulkan_GetInstanceExtensions(m_pSDLWindow, &unCount, extensions.data()) == 0)
	{
		GetEngine()->GetLogger()->Error("{}", SDL_GetError());
		return;
	}

	// Create Window Surface
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(m_pSDLWindow, pDevice->GetVkInstance(), &surface) == 0)
	{
		GetEngine()->GetLogger()->Error("Failed to create Vulkan surface. {}", SDL_GetError());
		return;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForVulkan(m_pSDLWindow);

	int w, h;
	SDL_Vulkan_GetDrawableSize(m_pSDLWindow, &w, &h);
	m_v2DrawableSize.x = w;
	m_v2DrawableSize.y = h;
	m_bWindowResized = true;

	MRenderView::InitializeForVulkan(pDevice, surface);

}

void SDLRenderView::UnbindSDLWindow()
{
	MRenderView::Release();

	if (m_pSDLWindow)
	{
		ImGui_ImplSDL2_Shutdown();
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = nullptr;
	}
}

void SDLRenderView::Render(MTaskNode* pNode)
{
	MORTY_UNUSED(pNode);
	
	if (GetMinimized())
	{
		return;
	}

	if (!m_pSDLWindow)
	{
		return;
	}

	if (m_bWindowResized)
	{
		MRenderView::Resize(m_v2DrawableSize);
		m_bWindowResized = false;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pDevice = pRenderSystem->GetDevice();
	MViewRenderTarget* pRenderTarget = GetNextRenderTarget();
	if (!pRenderTarget)
		return;

	MIRenderCommand* pRenderCommand = pDevice->CreateRenderCommand("MainEditor RenderCommand");
	if (!pRenderCommand)
		return;

	pRenderTarget->BindPrimaryCommand(pRenderCommand);

	pRenderCommand->RenderCommandBegin();

	ImGui_ImplSDL2_NewFrame(m_pSDLWindow);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();


	for (auto content : m_vContent)
	{
		content->OnRender(pRenderCommand);
	}


	// Rendering
	ImGui::Render();
	if (m_pImGuiRender)
	{
		m_pImGuiRender->Tick(0.0f);
		m_pImGuiRender->WaitTextureReady(pRenderCommand);
		pRenderCommand->BeginRenderPass(&pRenderTarget->renderPass);
		m_pImGuiRender->Render(pRenderCommand);
		pRenderCommand->EndRenderPass();
	}
	
	pRenderCommand->RenderCommandEnd();

	RenderMessageManager::GetInstance()->nDrawCallCount = pRenderCommand->GetDrawCallCount();

	Present(pRenderTarget);
}
