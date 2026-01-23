#pragma once

#include "Render/SceneViewer.h"
#include "Utility/IniConfig.h"
#include "View/MRenderView.h"

struct SDL_Window;

struct ImGuiContext;

namespace morty
{

class ImGuiRenderer;
class MNode;
class MScene;
class MInputEvent;
class MTaskNode;
class IRenderCommand;
class RenderViewContent
{
public:
    virtual void       OnRender(IRenderCommand* pRenderCommand) = 0;

    virtual void       OnResize(Vector2 size) = 0;

    virtual void       OnInput(MInputEvent* pEvent) = 0;

    virtual void       OnTick(float delta) = 0;

    virtual MTaskNode* GetRenderTask() { return nullptr; }
};

class SDLRenderView : public MRenderView
{
public:
    SDLRenderView() = default;

    virtual ~SDLRenderView() = default;

    void Initialize(MEngine* engine) override;

    void Release() override;

    void SetWindowTitle(const MString& title) { m_windowTitle = title; }

    void BindSDLWindow();

    void UnbindSDLWindow();

    int  GetViewWidth() const { return m_drawableSize.x; }

    int  GetViewHeight() const { return m_drawableSize.y; }

    bool GetMinimized() const { return m_windowMinimized; }

    bool GetClosed() const { return m_windowClosed; }

    void Input(MInputEvent* pEvent);

    bool MainLoop(MTaskNode* node);

    void Render(MTaskNode* node) override;

    void Resize(const int& nWidth, const int& nHeight);

    void AppendContent(RenderViewContent* pContent);

protected:
    MTaskNode* GetEditorRenderTask() const { return m_renderTask; }


private:
    ImGuiContext*                   m_imGUiContext = nullptr;

    Vector2                         m_drawableSize    = Vector2(80.0f, 48.0f);
    bool                            m_windowMinimized = false;
    bool                            m_windowResized   = false;
    bool                            m_windowClosed    = false;
    SDL_Window*                     m_sDLWindow       = nullptr;
    MTaskNode*                      m_updateTask      = nullptr;
    MTaskNode*                      m_renderTask      = nullptr;
    ImGuiRenderer*                  m_imGuiRender     = nullptr;

    std::vector<RenderViewContent*> m_content;

    MString                         m_windowTitle = "Morty";

    IniConfig                       m_IniConfig;

    static MString                  m_windowSettingFileName;
    static MString                  m_imGUISettingFileName;
    static MString                  m_imNodesSettingFileName;
};

}// namespace morty