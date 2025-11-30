#include "MainEditor.h"

#include "Utility/MGlobal.h"
#include "ImGuiFileDialog.h"
#include "ImGuizmo.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"


#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Utility/MRenderGlobal.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include <SDL_vulkan.h>

#endif
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "Input/MInputEvent.h"
#include "Material/MMaterial.h"
#include "Math/Matrix.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"
#include "RHI/IRenderCommand.h"
#include "Render/MDeferredRenderProgram.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"
#include "Utility/MTimer.h"
#include "Widget/GuizmoWidget.h"
#include "Widget/MainView.h"
#include "Widget/MenuBar.h"
#include "Widget/ModelImportView.h"
#include "Widget/NodeTreeView.h"
#include "Widget/PropertyViewManager.h"
#include "Widget/PropertyViewPanel.h"
#include "Widget/RenderGraphView.h"
#include "Widget/RenderSettingView.h"
#include "Widget/ResourceView.h"
#include "Widget/TaskGraphView.h"

using namespace morty;

MStringId MainEditor::m_renderProgramName    = MDeferredRenderProgram::GetClassTypeName();
MString   MainEditor::m_editorConfigFilePath = MString(MORTY_RESOURCE_PATH) + "/Editor/editor.ini";

bool      MainEditor::Initialize(MEngine* engine)
{
    m_engine = engine;

    m_IniConfig.LoadFromFile(m_editorConfigFilePath);

    MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();
    m_renderTask           = pMainGraph->AddNode<MTaskNode>(MStringId("Editor_Render"));
    m_renderTask->SetThreadType(METhreadType::ERenderThread);

    m_menuBar = new MenuBar();
    m_menuBar->Initialize(this);
    m_menuBar->LoadConfig(&m_IniConfig);

    m_childView.push_back(new NodeTreeView());
    m_childView.push_back(new ResourceView());
    m_childView.push_back(new ModelImportView());
    m_childView.push_back(new MainView());

    auto pTaskGraphView = new TaskGraphView("Task Graph");
    pTaskGraphView->SetTaskGraph(GetEngine()->GetMainGraph());
    m_childView.push_back(pTaskGraphView);

    m_renderGraphView = new RenderGraphView("Render Graph");
    m_childView.push_back(m_renderGraphView);

    for (BaseWidget* pChild: m_childView)
    {
        pChild->Initialize(this);
        pChild->LoadConfig(&m_IniConfig);
        m_menuBar->AddWidget(pChild);
    }

    // Create PropertyViewManager instead of individual PropertyView
    auto* propertyViewManager = new PropertyViewManager();
    m_childView.push_back(propertyViewManager);
    propertyViewManager->Initialize(this);
    propertyViewManager->LoadConfig(&m_IniConfig);

    return true;
}

void MainEditor::Release()
{

    if (m_sceneTexture)
    {
        DestroySceneViewer(m_sceneTexture);
        m_sceneTexture = nullptr;
    }

    // Save PropertyViewManager and its panels' configurations
    if (auto* propViewManager = FindWidget<PropertyViewManager>()) { propViewManager->SaveConfig(&m_IniConfig); }

    for (BaseWidget* pChild: m_childView)
    {
        // Skip PropertyViewManager as it's already saved
        if (dynamic_cast<PropertyViewManager*>(pChild)) { continue; }

        pChild->SaveConfig(&m_IniConfig);
        pChild->Release();
        delete pChild;
    }

    m_childView.clear();

    if (m_menuBar)
    {
        m_menuBar->SaveConfig(&m_IniConfig);
        m_menuBar->Release();
        delete m_menuBar;
        m_menuBar = nullptr;
    }

    m_IniConfig.Save(m_editorConfigFilePath);
}

MViewport* MainEditor::GetViewport() const { return m_sceneTexture->GetViewport(); }

void       MainEditor::SetScene(MScene* scene)
{
    if (m_scene == scene) { return; }

    m_scene = scene;

    if (m_sceneTexture)
    {
        DestroySceneViewer(m_sceneTexture);
        m_sceneTexture = nullptr;
    }

    m_sceneTexture = CreateSceneViewer("MainScene", m_scene);

    m_renderGraphView->SetRenderProgram(m_sceneTexture->GetRenderProgram());
}

void         MainEditor::OnResize(Vector2 size) { MORTY_UNUSED(size); }

void         MainEditor::OnInput(MInputEvent* pEvent) { m_sceneTexture->GetViewport()->Input(pEvent); }

void         MainEditor::OnTick(float delta) { m_scene->Tick(delta); }

SceneViewer* MainEditor::CreateSceneViewer(const MString& viewName, MScene* scene)
{
    auto sceneViewer = GetEngine()->FindSystem<MObjectSystem>()->CreateObject<SceneViewer>();
    sceneViewer->Initialize(viewName, scene, MainEditor::GetRenderProgramName());
    m_sceneViewer.insert(sceneViewer);

    sceneViewer->GetRenderTask()->ConnectTo(GetRenderTask());
    return sceneViewer;
}

void MainEditor::DestroySceneViewer(SceneViewer* pViewer)
{
    pViewer->DeleteLater();
    m_sceneViewer.erase(pViewer);
}

void MainEditor::UpdateSceneViewer(IRenderCommand* pRenderCommand)
{
    std::vector<MTexture*> vRenderTextures;
    for (const auto& pSceneViewer: m_sceneViewer)
    {
        pSceneViewer->UpdateTexture(pRenderCommand);

        if (auto texture = pSceneViewer->GetFinalOutputTexture()) { vRenderTextures.emplace_back(texture.get()); }
    }
}

void MainEditor::ShowView(BaseWidget* pView)
{
    bool bVisible = pView->GetVisible();

    if (bVisible)
    {
        if (ImGui::Begin(pView->GetName().c_str(), &bVisible, pView->GetWindowFlags())) { pView->Render(); }

        pView->SetVisible(bVisible);
        ImGui::End();
    }
    else if (pView->GetRenderInHidden()) { pView->Render(); }
}

Vector4 MainEditor::GetCurrentWidgetSize() const
{
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec2      v2RenderViewPos  = ImGui::GetWindowPos();
    ImVec2      v2RenderViewSize = ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

    v2RenderViewPos.x += style.WindowPadding.x;
    v2RenderViewPos.y += ImGui::GetItemRectSize().y;

    v2RenderViewSize.x -= style.WindowPadding.x * 2.0f;
    v2RenderViewSize.y -= (style.WindowPadding.y * 2.0f + ImGui::GetItemRectSize().y * 2.0f);

    return Vector4(v2RenderViewPos.x, v2RenderViewPos.y, v2RenderViewSize.x, v2RenderViewSize.y);
}

void MainEditor::OnRender(IRenderCommand* pRenderCommand)
{
    //update all scene viewer.
    UpdateSceneViewer(pRenderCommand);

    if (m_menuBar) { m_menuBar->Render(); }

    ImGui::DockSpaceOverViewport();

    for (BaseWidget* pBaseView: m_childView) { ShowView(pBaseView); }
}
