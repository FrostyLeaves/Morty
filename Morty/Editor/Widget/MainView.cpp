#include "MainView.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Basic/MTexture.h"
#include "Material/MMaterial.h"
#include "Basic/MViewport.h"
#include "Resource/MMaterialResource.h"

#include "imgui.h"
#include "ImGuizmo.h"

#include "GuizmoWidget.h"

#include "System/MSceneSystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Resource/MSkeletonResource.h"

#include "RenderProgram/MIRenderProgram.h"

#include "Main/MainEditor.h"
#include "Resource/MMeshResourceUtil.h"
#include "Utility/SelectionEntityManager.h"

using namespace morty;

MainView::MainView()
	: BaseWidget()
{
	m_strViewName = "MainView";
	m_bRenderInHidden = true;
}

void MainView::Render()
{
	Vector4 v4RenderViewSize = GetMainEditor()->GetCurrentWidgetSize();

	auto pTexture = GetMainEditor()->GetSceneTexture()->GetTexture();

    ImGui::SetNextWindowBgAlpha(0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("MainView", NULL,
     ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar
    );

    v4RenderViewSize = Vector4(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

    ImGui::Image({ pTexture, intptr_t(pTexture.get()), 0 }, ImVec2(v4RenderViewSize.z, v4RenderViewSize.w));


    ImGuizmo::SetRect(v4RenderViewSize.x, v4RenderViewSize.y, v4RenderViewSize.z, v4RenderViewSize.w);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    m_pGuizmoWidget->Render();

    ImGui::End();
    ImGui::PopStyleVar(2);

	GetMainEditor()->GetSceneTexture()->SetRect(Vector2i(v4RenderViewSize.x, v4RenderViewSize.y), Vector2i(v4RenderViewSize.z, v4RenderViewSize.w));
}

void MainView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);

	m_pGuizmoWidget = new GuizmoWidget();
	m_pGuizmoWidget->Initialize(GetMainEditor());
	AddWidget(m_pGuizmoWidget);
}

void MainView::Release()
{
	m_pGuizmoWidget->Release();
	delete m_pGuizmoWidget;
	m_pGuizmoWidget = nullptr;

}
