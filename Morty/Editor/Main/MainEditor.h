#pragma once

#include "Render/SceneViewer.h"
#include "Variant/MVariant.h"

#include "SDLRenderView.h"
#include "Widget/RenderSettingView.h"

namespace morty
{

class MainView;
class GuizmoWidget;
class MScene;
class MViewport;
class ImGuiRenderer;
class BaseWidget;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class TaskGraphView;
class ModelImportView;
class RenderGraphView;
class MessageWidget;
class MInputEvent;
class MTaskNode;
class MenuBar;

class MainEditor : public RenderViewContent
{
public:
    MainEditor() = default;

    virtual ~MainEditor() = default;

    bool                                       Initialize(MEngine* engine);

    void                                       Release();

    [[nodiscard]] MEngine*                     GetEngine() const { return m_engine; }

    void                                       SetScene(MScene* scene);

    [[nodiscard]] MScene*                      GetScene() const { return m_scene; }

    [[nodiscard]] MViewport*                   GetViewport() const;

    [[nodiscard]] std::shared_ptr<SceneViewer> GetSceneTexture() const { return m_sceneTexture; }
    [[nodiscard]] RenderGraphView*             GetRenderGraphView() const { return m_renderGraphView; }

    void                                       OnResize(morty::Vector2 size) override;

    void                                       OnRender(IRenderCommand* pRenderCommand) override;

    void                                       OnInput(MInputEvent* pEvent) override;

    void                                       OnTick(float fDelta) override;

    static morty::MStringId                    GetRenderProgramName() { return m_renderProgramName; }

    MTaskNode*                                 GetRenderTask() override { return m_renderTask; }

    std::shared_ptr<SceneViewer>               CreateSceneViewer(const MString& viewName, MScene* scene);

    void                                       DestroySceneViewer(std::shared_ptr<SceneViewer> pViewer);

    [[nodiscard]] morty::Vector4               GetCurrentWidgetSize() const;

    template<class TYPE> TYPE*                 FindWidget() const;

protected:
    void UpdateSceneViewer(IRenderCommand* pRenderCommand);

    void ShowView(BaseWidget* pView);

private:
    MEngine*                               m_engine = nullptr;
    MScene*                                m_scene  = nullptr;
    std::vector<BaseWidget*>               m_childView;
    MenuBar*                               m_menuBar = nullptr;
    std::set<std::shared_ptr<SceneViewer>> m_sceneViewer;

    Vector4                                m_renderViewSize = Vector4(0, 0, 32, 32);

    RenderGraphView*                       m_renderGraphView = nullptr;
    std::shared_ptr<SceneViewer>           m_sceneTexture    = nullptr;
    MTaskNode*                             m_renderTask      = nullptr;

    IniConfig                              m_IniConfig;

    static MStringId                       m_renderProgramName;
    static MString                         m_editorConfigFilePath;
};

template<class TYPE> TYPE* MainEditor::FindWidget() const
{
    for (auto widget: m_childView)
    {
        if (auto result = dynamic_cast<TYPE*>(widget)) { return result; }
    }

    return nullptr;
}

}// namespace morty