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
class ImGuiRenderable;
class BaseWidget;
class NodeTreeView;
class PropertyView;
class MaterialView;
class ResourceView;
class TaskGraphView;
class ModelConvertView;
class RenderGraphView;
class MessageWidget;
class MInputEvent;
class MTaskNode;

class MainEditor : public RenderViewContent
{
public:
    MainEditor() = default;

    virtual ~MainEditor() = default;

    bool                                       Initialize(MEngine* pEngine);

    void                                       Release();

    [[nodiscard]] MEngine*                     GetEngine() const { return m_engine; }

    void                                       SetScene(MScene* pScene);

    [[nodiscard]] MScene*                      GetScene() const { return m_scene; }

    [[nodiscard]] MViewport*                   GetViewport() const;

    [[nodiscard]] std::shared_ptr<SceneViewer> GetSceneTexture() const { return m_sceneTexture; }
    [[nodiscard]] RenderGraphView*             GetRenderGraphView() const { return m_renderGraphView; }

    void                                       OnResize(morty::Vector2 size) override;

    void                                       OnRender(MIRenderCommand* pRenderCommand) override;

    void                                       OnInput(MInputEvent* pEvent) override;

    void                                       OnTick(float fDelta) override;

    static morty::MString                      GetRenderProgramName() { return m_renderProgramName; }

    MTaskNode*                                 GetRenderTask() override { return m_renderTask; }

    std::shared_ptr<SceneViewer>               CreateSceneViewer(const MString& viewName, MScene* pScene);

    void                                       DestroySceneViewer(std::shared_ptr<SceneViewer> pViewer);

    [[nodiscard]] morty::Vector4               GetCurrentWidgetSize() const;

protected:
    void UpdateSceneViewer(MIRenderCommand* pRenderCommand);

    void ShowMenu();

    void ShowShadowMapView();

    void ShowView(BaseWidget* pView);

    void ShowDialog();

private:
    MEngine*                               m_engine = nullptr;
    MScene*                                m_scene  = nullptr;
    std::vector<BaseWidget*>               m_childView;
    std::set<std::shared_ptr<SceneViewer>> m_sceneViewer;

    bool                                   m_showRenderView = false;
    bool                                   m_showDebugView  = false;

    Vector4                                m_renderViewSize = Vector4(0, 0, 32, 32);

    RenderGraphView*                       m_renderGraphView = nullptr;
    std::shared_ptr<SceneViewer>           m_sceneTexture    = nullptr;
    MTaskNode*                             m_renderTask      = nullptr;

    IniConfig                              m_IniConfig;

    static MString                         m_renderProgramName;
    static MString                         m_editorConfigFilePath;
};

}// namespace morty