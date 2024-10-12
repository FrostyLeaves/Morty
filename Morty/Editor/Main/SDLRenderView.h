#pragma once

#include "Render/SceneTexture.h"
#include "Utility/IniConfig.h"
#include "View/MRenderView.h"

struct SDL_Window;

struct ImGuiContext;

namespace morty
{

class ImGuiRenderable;
class MNode;
class MScene;
class MInputEvent;
class MTaskNode;
class MIRenderCommand;
class RenderViewContent
{
public:
    virtual void       OnRender(MIRenderCommand* pRenderCommand) = 0;

    virtual void       OnResize(Vector2 size) = 0;

    virtual void       OnInput(MInputEvent* pEvent) = 0;

    virtual void       OnTick(float fDelta) = 0;

    virtual MTaskNode* GetRenderTask() { return nullptr; }
};

class SDLRenderView : public MRenderView
{
public:
    SDLRenderView() = default;

    virtual ~SDLRenderView() = default;

    void Initialize(MEngine* pEngine) override;

    void Release() override;

    void BindSDLWindow();

    void UnbindSDLWindow();

    int  GetViewWidth() const { return m_drawableSize.x; }

    int  GetViewHeight() const { return m_drawableSize.y; }

    bool GetMinimized() const { return m_windowMinimized; }

    bool GetClosed() const { return m_windowClosed; }

    void Input(MInputEvent* pEvent);

    bool MainLoop(MTaskNode* pNode);

    void Render(MTaskNode* pNode) override;

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
    MTaskNode*                      m_renderTask      = nullptr;
    ImGuiRenderable*                m_imGuiRender     = nullptr;

    std::vector<RenderViewContent*> m_content;

    IniConfig                       m_IniConfig;

    static MString                  m_windowSettingFileName;
    static MString                  m_imGUISettingFileName;
};

}// namespace morty