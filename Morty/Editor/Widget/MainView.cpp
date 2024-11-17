#include "MainView.h"

#include "imgui.h"

#include "ImGuizmo.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "GuizmoWidget.h"
#include "Main/MainEditor.h"
#include "Material/MMaterial.h"
#include "Object/MObject.h"
#include "Render/MIRenderProgram.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "Resource/MSkeletonResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "System/MSceneSystem.h"
#include "Utility/RenderMessageManager.h"
#include "Utility/SelectionEntityManager.h"
#include "Utility/SingletonInstance.h"
#include "Widget/RenderGraphView.h"


using namespace morty;

MainView::MainView()
    : BaseWidget()
{
    m_strViewName    = "MainView";
    m_renderInHidden = true;
}

void MainView::Render()
{
    Vector4 v4RenderViewSize = GetMainEditor()->GetCurrentWidgetSize();

    ImGui::SetNextWindowBgAlpha(0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin(
            "MainView",
            NULL,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar
    );

    v4RenderViewSize =
            Vector4(ImGui::GetWindowPos().x,
                    ImGui::GetWindowPos().y,
                    ImGui::GetContentRegionAvail().x,
                    ImGui::GetContentRegionAvail().y);

    if (auto pTexture = GetMainEditor()->GetSceneTexture()->GetFinalOutputTexture())
    {
        ImGui::Image(
                {pTexture, intptr_t(pTexture.get()), static_cast<size_t>(m_textureIdx)},
                ImVec2(v4RenderViewSize.z, v4RenderViewSize.w)
        );
    }

    ImGuizmo::SetRect(v4RenderViewSize.x, v4RenderViewSize.y, v4RenderViewSize.z, v4RenderViewSize.w);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    m_guizmoWidget->Render();
    DrawMessage();

    ImGui::End();
    ImGui::PopStyleVar(2);

    GetMainEditor()->GetSceneTexture()->SetRect(
            Vector2i(v4RenderViewSize.x, v4RenderViewSize.y),
            Vector2i(v4RenderViewSize.z, v4RenderViewSize.w)
    );
}

void MainView::Initialize(MainEditor* pMainEditor)
{
    BaseWidget::Initialize(pMainEditor);

    m_guizmoWidget = new GuizmoWidget();
    m_guizmoWidget->Initialize(GetMainEditor());
    AddWidget(m_guizmoWidget);
}

void MainView::Release()
{
    m_guizmoWidget->Release();
    MORTY_SAFE_DELETE(m_guizmoWidget);
}

void MainView::DrawMessage()
{
    const float DISTANCE = 10.0f;
    static int  corner   = 0;
    if (corner != -1)
    {
        ImVec2 window_pos =
                ImVec2((corner & 1) ? ImGui::GetWindowSize().x - DISTANCE : DISTANCE,
                       (corner & 2) ? ImGui::GetWindowSize().y - DISTANCE : DISTANCE + 24);// 24 is title bar height,
        window_pos = ImVec2(ImGui::GetWindowPos().x + window_pos.x, ImGui::GetWindowPos().y + window_pos.y);

        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowSize(ImVec2(300, 0));
    }
    ImGui::SetNextWindowBgAlpha(0.35f);// Transparent background
    if (ImGui::Begin(
                "Message",
                &m_visiable,
                (corner != -1 ? ImGuiWindowFlags_NoMove : ImGuiWindowFlags_NoTitleBar) | ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
        ))
    {
        int nCurrentFps = (int) round(GetEngine()->GetFPS() / 5) * 5;

        ImGui::Columns(2);

        ImGui::Text("FPS");
        ImGui::NextColumn();
        ImGui::Text("%d", nCurrentFps);
        ImGui::NextColumn();

        ImGui::Text("Draw Call");
        ImGui::NextColumn();
        ImGui::Text("%zu", RenderMessageManager::GetInstance()->nDrawCallCount);
        ImGui::NextColumn();

        if (auto pTexture = GetMainEditor()->GetSceneTexture()->GetFinalOutputTexture())
        {
            auto max     = pTexture->GetLayer() - 1;
            m_textureIdx = std::clamp<int>(m_textureIdx, 0, max);
            ImGui::Text("Texture Idx");
            ImGui::NextColumn();
            ImGui::SliderInt("", &m_textureIdx, 0, max);
            ImGui::NextColumn();
        }

        ImGui::Columns(1);

        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
            if (ImGui::MenuItem("Close")) m_visiable = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}
