#include "MessageWidget.h"

#include "Engine/MEngine.h"
#include "Utility/RenderMessageManager.h"
#include "imgui.h"

using namespace morty;

MessageWidget::MessageWidget()
    : BaseWidget()
{
    m_strViewName = "Message";
}

void MessageWidget::Render()
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
        ImGui::Text("FPS: %d", nCurrentFps);

        ImGui::Text("Draw Call Count: %zu", RenderMessageManager::GetInstance()->nDrawCallCount);


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

void MessageWidget::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void MessageWidget::Release() {}
