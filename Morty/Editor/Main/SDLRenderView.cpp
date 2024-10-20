#include "SDLRenderView.h"

#include "Utility/MGlobal.h"
#include "SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "ImGuizmo.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Utility/MRenderGlobal.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include <SDL_vulkan.h>

#endif


#include "Engine/MEngine.h"
#include "Input/MInputEvent.h"
#include "Mesh/MMesh.h"
#include "RHI/MRenderCommand.h"
#include "Scene/MScene.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"

#include "Component/MRenderMeshComponent.h"
#include "Render/ImGui/imnodes.h"
#include "Render/ImGuiRenderable.h"

#include "System/MInputSystem.h"
#include "System/MRenderSystem.h"

#include "Utility/RenderMessageManager.h"

using namespace morty;

MString SDLRenderView::m_windowSettingFileName  = MString(MORTY_RESOURCE_PATH) + "/Editor/window.ini";
MString SDLRenderView::m_imGUISettingFileName   = MString(MORTY_RESOURCE_PATH) + "/Editor/imgui.ini";
MString SDLRenderView::m_imNodesSettingFileName = MString(MORTY_RESOURCE_PATH) + "/Editor/imnodes.ini";


void    SDLRenderView::Initialize(MEngine* pEngine)
{
    m_IniConfig.LoadFromFile(m_windowSettingFileName);
    auto windowSize = m_IniConfig.GetValue<Vector2i>("Window", "Size");

    MRenderView::Initialize(pEngine);
    MRenderView::InitSize(std::max(1, windowSize.x), std::max(1, windowSize.y));

    //Setup ImGui
    m_imGUiContext = ImGui::CreateContext();
    ImGuizmo::SetImGuiContext(m_imGUiContext);
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();

    ImGuiStyle& style                    = ImGui::GetStyle();
    style.WindowRounding                 = 0.0f;
    style.WindowPadding                  = ImVec2(2.0f, 2.0f);
    style.ItemSpacing.x                  = 2.0f;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.WantSaveIniSettings = true;

    ImGui::LoadIniSettingsFromDisk(m_imGUISettingFileName.c_str());
    ImNodes::LoadCurrentEditorStateFromIniFile(m_imNodesSettingFileName.c_str());


    m_imGuiRender = new ImGuiRenderable(pEngine);
    m_imGuiRender->Initialize();

    MTaskGraph* pMainGraph  = GetEngine()->GetMainGraph();
    MTaskNode*  pEditorTask = pMainGraph->AddNode<MTaskNode>(MStringId("Window_Update"));
    pEditorTask->SetThreadType(METhreadType::ECurrentThread);
    pEditorTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(SDLRenderView::MainLoop, this));

    m_renderTask = pMainGraph->AddNode<MTaskNode>(MStringId("Window_Render"));
    m_renderTask->SetThreadType(METhreadType::ERenderThread);
    m_renderTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(SDLRenderView::Render, this));
}

void SDLRenderView::Release()
{
    ImGui::SaveIniSettingsToDisk(m_imGUISettingFileName.c_str());
    ImNodes::SaveCurrentEditorStateToIniFile(m_imNodesSettingFileName.c_str());

    m_IniConfig.SetValue("Window", "Size", Vector2i(GetWidth(), GetHeight()));
    m_IniConfig.Save(m_windowSettingFileName);

    if (m_imGuiRender)
    {
        m_imGuiRender->Release();
        delete m_imGuiRender;
        m_imGuiRender = nullptr;
    }

    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    UnbindSDLWindow();

    MRenderView::Release();
}

void SDLRenderView::Resize(const int& nWidth, const int& nHeight)
{
    if (nWidth == 0 || nHeight == 0) { return; }

    int w, h;
    SDL_Vulkan_GetDrawableSize(m_sDLWindow, &w, &h);

    if (w == 0 || h == 0) return;

    m_drawableSize.x = w;
    m_drawableSize.y = h;
    m_windowResized  = true;

    if (m_windowResized)//TODO android surface only active in current call stack
    {
        MRenderView::Resize(m_drawableSize);
        m_windowResized = false;
    }


    for (auto content: m_content) { content->OnResize(m_drawableSize); }
}

void SDLRenderView::AppendContent(RenderViewContent* pContent)
{
    if (auto pTaskNode = pContent->GetRenderTask()) { pTaskNode->ConnectTo(GetEditorRenderTask()); }

    m_content.push_back(pContent);
}

void SDLRenderView::Input(MInputEvent* pEvent)
{
    if (MInputSystem* pInputSystem = GetEngine()->FindSystem<MInputSystem>()) { pInputSystem->Input(pEvent); }

    for (auto content: m_content) { content->OnInput(pEvent); }
}

bool SDLRenderView::MainLoop(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    SDL_Event event;
    bool      bClosed = false;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) bClosed = true;

        if (event.type == SDL_WINDOWEVENT && event.window.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) { Resize(event.window.data1, event.window.data2); }

            else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                m_windowMinimized = true;
            else if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
                m_windowMinimized = false;
            else if (event.window.event == SDL_WINDOWEVENT_RESTORED)
                m_windowMinimized = false;

            else if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                bClosed = true;
        }
        else if (event.type == SDL_KEYDOWN && event.key.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::DOWN);
            Input(&e);
        }
        else if (event.type == SDL_KEYUP && event.key.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::UP);
            Input(&e);
        }
        else if (event.type == SDL_TEXTEDITING && event.key.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            MKeyBoardInputEvent e(event.key.keysym.sym, MEKeyState::DOWN);
            Input(&e);
        }

        else if (event.type == SDL_MOUSEBUTTONUP && event.button.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            MMouseInputEvent::MEMouseDownButton type;
            if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
            if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
            if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

            MMouseInputEvent e(type, MMouseInputEvent::ButtonUp);
            Input(&e);
        }

        else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            MMouseInputEvent::MEMouseDownButton type;
            if (event.button.button == 0x01) type = MMouseInputEvent::LeftButton;
            if (event.button.button == 0x02) type = MMouseInputEvent::ScrollButton;
            if (event.button.button == 0x03) type = MMouseInputEvent::RightButton;

            MMouseInputEvent e(type, MMouseInputEvent::ButtonDown);
            Input(&e);
        }

        else if (event.type == SDL_MOUSEMOTION && event.button.windowID == SDL_GetWindowID(m_sDLWindow))
        {
            Vector2        new_pos(event.button.x, event.button.y);

            static Vector2 prev_pos = Vector2(-1, -1);
            if (prev_pos.x == -1 && prev_pos.y == -1) { prev_pos = new_pos; }

            MMouseInputEvent e(new_pos, new_pos - prev_pos);
            prev_pos = new_pos;

            Input(&e);
        }
    }

    for (auto content: m_content) { content->OnTick(GetEngine()->getTickDelta()); }


    m_windowClosed = bClosed;
    return !bClosed;
}

void SDLRenderView::BindSDLWindow()
{
    if (m_sDLWindow) { UnbindSDLWindow(); }

    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    MVulkanDevice* pDevice       = dynamic_cast<MVulkanDevice*>(pRenderSystem->GetDevice());

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
    m_sDLWindow                  = SDL_CreateWindow(
            "Morty Editor",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            GetWidth(),
            GetHeight(),
            window_flags
    );

#elif defined(MORTY_IOS) || defined(MORTY_ANDROID)
    m_sDLWindow = SDL_CreateWindow(
            NULL,
            0,
            0,
            320,
            480,
            SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_ALLOW_HIGHDPI
    );
#endif

    if (nullptr == m_sDLWindow) { GetEngine()->GetLogger()->Error("SDL_CreateWindow failed. {}", SDL_GetError()); }


    uint32_t                 unCount = 0;
    std::vector<const char*> extensions;
    SDL_Vulkan_GetInstanceExtensions(m_sDLWindow, &unCount, nullptr);

    extensions.resize(unCount);

    if (SDL_Vulkan_GetInstanceExtensions(m_sDLWindow, &unCount, extensions.data()) == 0)
    {
        GetEngine()->GetLogger()->Error("{}", SDL_GetError());
        return;
    }

    // Create Window Surface
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(m_sDLWindow, pDevice->GetVkInstance(), &surface) == 0)
    {
        GetEngine()->GetLogger()->Error("Failed to create Vulkan surface. {}", SDL_GetError());
        return;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForVulkan(m_sDLWindow);

    int w, h;
    SDL_Vulkan_GetDrawableSize(m_sDLWindow, &w, &h);
    m_drawableSize.x = w;
    m_drawableSize.y = h;
    m_windowResized  = true;

    MRenderView::InitializeForVulkan(pDevice, surface);
}

void SDLRenderView::UnbindSDLWindow()
{
    MRenderView::Release();

    if (m_sDLWindow)
    {
        ImGui_ImplSDL2_Shutdown();
        SDL_DestroyWindow(m_sDLWindow);
        m_sDLWindow = nullptr;
    }
}

void SDLRenderView::Render(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    if (GetMinimized()) { return; }

    if (!m_sDLWindow) { return; }

    if (m_windowResized)
    {
        MRenderView::Resize(m_drawableSize);
        m_windowResized = false;
    }

    MRenderSystem*     pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    MIDevice*          pDevice       = pRenderSystem->GetDevice();
    MViewRenderTarget* pRenderTarget = GetNextRenderTarget();
    if (!pRenderTarget) return;

    MIRenderCommand* pRenderCommand = pDevice->CreateRenderCommand("MainEditor RenderCommand");
    if (!pRenderCommand) return;

    pRenderTarget->BindPrimaryCommand(pRenderCommand);

    pRenderCommand->RenderCommandBegin();

    ImGui::SetCurrentContext(m_imGUiContext);
    ImGui_ImplSDL2_NewFrame(m_sDLWindow);
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();


    for (auto content: m_content) { content->OnRender(pRenderCommand); }


    // Rendering
    ImGui::Render();
    if (m_imGuiRender)
    {
        m_imGuiRender->Tick(0.0f);
        m_imGuiRender->WaitTextureReady(pRenderCommand);
        pRenderCommand->BeginRenderPass(&pRenderTarget->renderPass);
        m_imGuiRender->Render(pRenderCommand);
        pRenderCommand->EndRenderPass();
    }

    pRenderCommand->RenderCommandEnd();

    RenderMessageManager::GetInstance()->nDrawCallCount = pRenderCommand->GetDrawCallCount();

    Present(pRenderTarget);
}
