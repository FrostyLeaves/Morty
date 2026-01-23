#include "SimpleSceneContent.h"

#include "imgui.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Input/MInputEvent.h"
#include "RHI/IRenderCommand.h"
#include "Render/MRenderGraphProgram.h"
#include "Render/SceneViewer.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

bool SimpleSceneContent::Initialize(MEngine* engine, MScene* scene)
{
    m_engine = engine;
    m_scene  = scene;

    MTaskGraph* pMainGraph = m_engine->GetMainGraph();
    m_renderTask           = pMainGraph->AddNode<MTaskNode>(MStringId("SimpleScene_Render"));
    m_renderTask->SetThreadType(METhreadType::ERenderThread);

    auto* objectSystem = m_engine->GetSystem<MObjectSystem>();
    m_sceneViewer      = objectSystem->CreateObject<SceneViewer>();
    m_sceneViewer->Initialize("SimpleView", m_scene, MRenderGraphProgram::GetClassTypeName());

    m_sceneViewer->GetRenderTask()->ConnectTo(m_renderTask);

    return true;
}

void SimpleSceneContent::Release()
{
    if (m_sceneViewer)
    {
        m_sceneViewer->DeleteLater();
        m_sceneViewer = nullptr;
    }

    if (m_renderTask)
    {
        MTaskGraph* pMainGraph = m_engine->GetMainGraph();
        if (pMainGraph)
        {
            pMainGraph->DestroyNode(m_renderTask);
            m_renderTask = nullptr;
        }
    }
}

void SimpleSceneContent::OnRender(IRenderCommand* pRenderCommand)
{
    m_sceneViewer->Render(pRenderCommand);

    auto texture = m_sceneViewer->GetFinalOutputTexture();
    if (!texture) { return; }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_viewSize.x, m_viewSize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("SceneView", nullptr, windowFlags)) { ImGui::Image({texture, intptr_t(texture.get()), 0}, ImVec2(m_viewSize.x, m_viewSize.y)); }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void SimpleSceneContent::OnResize(Vector2 size)
{
    m_viewSize = size;
    m_sceneViewer->SetRect(Vector2i(0, 0), Vector2i(static_cast<int>(size.x), static_cast<int>(size.y)));
}

void SimpleSceneContent::OnInput(MInputEvent* pEvent)
{
    if (m_sceneViewer && m_sceneViewer->GetViewport()) { m_sceneViewer->GetViewport()->Input(pEvent); }
}

void SimpleSceneContent::OnTick(float delta)
{
    m_scene->Tick(delta);
    m_sceneViewer->Tick(delta);
}

MTaskNode* SimpleSceneContent::GetRenderTask() { return m_renderTask; }
