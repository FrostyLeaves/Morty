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
#include "Utility/SelectionEntityManager.h"
#include "Widget/MessageWidget.h"
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
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav |
                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoScrollbar
    );

    v4RenderViewSize =
            Vector4(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()
            );

    if (auto pTexture = GetMainEditor()->GetRenderGraphView()->GetFinalOutput())
    {
        ImGui::Image({pTexture, intptr_t(pTexture.get()), 0}, ImVec2(v4RenderViewSize.z, v4RenderViewSize.w));
    }

    ImGuizmo::SetRect(v4RenderViewSize.x, v4RenderViewSize.y, v4RenderViewSize.z, v4RenderViewSize.w);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    m_guizmoWidget->Render();
    m_messageWidget->Render();

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

    m_messageWidget = new MessageWidget();
    m_messageWidget->Initialize(GetMainEditor());
    AddWidget(m_messageWidget);
}

void MainView::Release()
{
    m_guizmoWidget->Release();
    MORTY_SAFE_DELETE(m_guizmoWidget);

    m_messageWidget->Release();
    MORTY_SAFE_DELETE(m_messageWidget);
}
