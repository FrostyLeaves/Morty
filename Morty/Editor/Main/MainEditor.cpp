#include "MainEditor.h"

#include "Utility/MGlobal.h"
#include "ImGuiFileDialog.h"
#include "ImGuizmo.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"

#include "SDL.h"

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
#include "RHI/MRenderCommand.h"
#include "RenderProgram/MDeferredRenderProgram.h"
#include "Scene/MScene.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"
#include "Utility/MTimer.h"
#include "Widget/GuizmoWidget.h"
#include "Widget/MainView.h"
#include "Widget/MaterialView.h"
#include "Widget/MessageWidget.h"
#include "Widget/ModelConvertView.h"
#include "Widget/NodeTreeView.h"
#include "Widget/PropertyView.h"
#include "Widget/RenderSettingView.h"
#include "Widget/ResourceView.h"
#include "Widget/TaskGraphView.h"

using namespace morty;

MString MainEditor::m_renderProgramName    = MDeferredRenderProgram::GetClassTypeName();
MString MainEditor::m_editorConfigFilePath = MString(MORTY_RESOURCE_PATH) + "/Editor/editor.ini";

bool    MainEditor::Initialize(MEngine* pEngine)
{
    m_engine = pEngine;

    m_IniConfig.LoadFromFile(m_editorConfigFilePath);

    MTaskGraph* pMainGraph = GetEngine()->GetMainGraph();
    m_renderTask           = pMainGraph->AddNode<MTaskNode>(MStringId("Editor_Render"));
    m_renderTask->SetThreadType(METhreadType::ERenderThread);

    m_childView.push_back(new NodeTreeView());
    m_childView.push_back(new PropertyView());
    m_childView.push_back(new MaterialView());
    m_childView.push_back(new ResourceView());
    m_childView.push_back(new ModelConvertView());
    m_childView.push_back(new MessageWidget());
    m_childView.push_back(new MainView());

    auto pTaskGraphView = new TaskGraphView("Task Graph");
    pTaskGraphView->SetTaskGraph(GetEngine()->GetMainGraph());
    m_childView.push_back(pTaskGraphView);


    m_renderGraphView = new TaskGraphView("Render Graph");
    m_childView.push_back(m_renderGraphView);

    m_renderSettingView = new RenderSettingView();
    m_childView.push_back(m_renderSettingView);


    for (BaseWidget* pChild: m_childView)
    {
        pChild->Initialize(this);
        pChild->LoadConfig(&m_IniConfig);
    }

    return true;
}

void MainEditor::Release()
{

    if (m_sceneTexture)
    {
        DestroySceneViewer(m_sceneTexture);
        m_sceneTexture = nullptr;
    }

    for (BaseWidget* pChild: m_childView)
    {
        pChild->SaveConfig(&m_IniConfig);
        pChild->Release();
        delete pChild;
    }

    m_childView.clear();

    m_IniConfig.Save(m_editorConfigFilePath);
}

MViewport* MainEditor::GetViewport() const { return m_sceneTexture->GetViewport(); }

void       MainEditor::SetScene(MScene* pScene)
{
    if (m_scene == pScene) { return; }

    m_scene = pScene;

    if (m_sceneTexture)
    {
        DestroySceneViewer(m_sceneTexture);
        m_sceneTexture = nullptr;
    }

    m_sceneTexture = CreateSceneViewer("MainScene", m_scene);

    m_renderGraphView->SetTaskGraph(m_sceneTexture->GetRenderProgram()->GetRenderGraph());
    m_renderSettingView->SetRenderGraph(
            m_sceneTexture->GetRenderProgram()->GetRenderGraph()->DynamicCast<MRenderGraph>()->GetRenderGraphSetting()
    );
}

void                         MainEditor::OnResize(Vector2 size) { MORTY_UNUSED(size); }

void                         MainEditor::OnInput(MInputEvent* pEvent) { m_sceneTexture->GetViewport()->Input(pEvent); }

void                         MainEditor::OnTick(float fDelta) { m_scene->Tick(fDelta); }

std::shared_ptr<SceneViewer> MainEditor::CreateSceneViewer(const MString& viewName, MScene* pScene)
{
    std::shared_ptr<SceneViewer> pSceneViewer = std::make_shared<SceneViewer>();
    pSceneViewer->Initialize(viewName, pScene, MainEditor::GetRenderProgramName());
    m_sceneViewer.insert(pSceneViewer);

    pSceneViewer->GetRenderTask()->ConnectTo(GetRenderTask());
    return pSceneViewer;
}

void MainEditor::DestroySceneViewer(std::shared_ptr<SceneViewer> pViewer)
{
    pViewer->Release();
    m_sceneViewer.erase(pViewer);
}

void MainEditor::UpdateSceneViewer(MIRenderCommand* pRenderCommand)
{
    std::vector<MTexture*> vRenderTextures;
    for (auto pSceneViewer: m_sceneViewer)
    {
        pSceneViewer->UpdateTexture(pRenderCommand);

        if (std::shared_ptr<MTexture> pRenderTexture = pSceneViewer->GetTexture())
        {
            vRenderTextures.push_back(pRenderTexture.get());
        }
    }

    pRenderCommand->AddRenderToTextureBarrier(vRenderTextures, METextureBarrierStage::EPixelShaderSample);
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

            if (ImGui::MenuItem("Save", "")) {}

            if (ImGui::MenuItem("Save as", ""))
            {
                ImGuiFileDialog::Instance()->OpenModal("Save As", "Open", "entity\0\0", "new");
            }


            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Render", "", &m_showRenderView)) {}
            if (ImGui::MenuItem("DebugTexture", "", &m_showDebugView)) {}

            for (BaseWidget* pView: m_childView)
            {
                bool bVisible = pView->GetVisible();
                if (ImGui::MenuItem(pView->GetName().c_str(), "", &bVisible)) {}
                pView->SetVisible(bVisible);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Convert model")) {}

            if (ImGui::MenuItem("Load model")) {}

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tool"))
        {
            if (ImGui::MenuItem("Snip shot"))
            {
                auto t = std::time(nullptr);
                tm   outtm;
                MORTY_ASSERT(0 == MTimer::LocalTime(t, outtm));
                std::ostringstream oss;
                oss << std::put_time(&outtm, "%d-%m-%Y %H-%M-%S");
                auto str = oss.str();

                if (m_sceneTexture) { m_sceneTexture->Snapshot("./Snipshot-" + str + ".png"); }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void MainEditor::ShowShadowMapView()
{
    if (!m_showDebugView) return;

    if (ImGui::Begin("DebugView", &m_showDebugView))
    {
        std::vector<std::shared_ptr<MTexture>> vTexture = m_sceneTexture->GetAllOutputTexture();
        if (!vTexture.empty())
        {
            size_t nImageSize = 0;
            for (std::shared_ptr<MTexture> pTexture: vTexture)
            {
                if (pTexture) { nImageSize += pTexture->GetSize().z; }
            }

            // n * n
            Vector4 v4Rect    = GetCurrentWidgetSize();
            size_t  nRowCount = std::ceil(std::sqrt(nImageSize));
            Vector2 v2Size    = Vector2((v4Rect.z) / nRowCount, (v4Rect.w) / nRowCount);

            ImGui::Columns(static_cast<int>(nRowCount));
            for (size_t nTexIdx = 0; nTexIdx < vTexture.size(); ++nTexIdx)
            {
                for (size_t nLayerIdx = 0; nLayerIdx < static_cast<size_t>(vTexture[nTexIdx]->GetSize().z); ++nLayerIdx)
                {
                    ImGui::Image(
                            {vTexture[nTexIdx], intptr_t(vTexture[nTexIdx].get()), nLayerIdx},
                            ImVec2(v2Size.x, v2Size.y)
                    );
                    ImGui::Text("%s (%d)", vTexture[nTexIdx]->GetName().c_str(), static_cast<int>(nLayerIdx));

                    ImGui::NextColumn();
                }
            }
            ImGui::Columns(1);
        }
    }

    ImGui::End();
}

void MainEditor::ShowView(BaseWidget* pView)
{
    bool bVisible = pView->GetVisible();

    if (bVisible)
    {
        if (ImGui::Begin(pView->GetName().c_str(), &bVisible, ImGuiWindowFlags_NoCollapse)) { pView->Render(); }

        pView->SetVisible(bVisible);
        ImGui::End();
    }
    else if (pView->GetRenderInHidden()) { pView->Render(); }
}

void MainEditor::ShowDialog()
{
    if (ImGuiFileDialog::Instance()->Display("OpenFile"))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            std::map<std::string, std::string>&& files = ImGuiFileDialog::Instance()->GetSelection();
        }
        ImGuiFileDialog::Instance()->Close();
    }


    if (ImGuiFileDialog::Instance()->Display("Save As"))
    {
        if (ImGuiFileDialog::Instance()->IsOk() == true)
        {
            std::string strFilePathName    = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string strCurrentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("Convert Model")) {}
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

void MainEditor::OnRender(MIRenderCommand* pRenderCommand)
{
    if (m_sceneTexture)
    {
        //		m_sceneTexture->SetRect(Vector2(m_renderViewSize.x, m_renderViewSize.y), Vector2(m_renderViewSize.z, m_renderViewSize.w));
    }

    //update all scene viewer.
    UpdateSceneViewer(pRenderCommand);

    ShowMenu();

    ShowShadowMapView();

    ImGui::DockSpaceOverViewport();

    for (BaseWidget* pBaseView: m_childView) { ShowView(pBaseView); }


    ShowDialog();
}
