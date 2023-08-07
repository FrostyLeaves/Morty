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

MainView::MainView()
	: BaseWidget()
{
	m_strViewName = "MainView";
	m_bRenderInHidden = true;
}

void MainView::Render()
{
	Vector4 m_v4RenderViewSize = GetMainEditor()->GetCurrentWidgetSize();

	auto pTexture = GetMainEditor()->GetSceneTexture()->GetTexture(0);



	if (GetVisible())
	{
		ImGuizmo::SetRect(m_v4RenderViewSize.x, m_v4RenderViewSize.y, m_v4RenderViewSize.z, m_v4RenderViewSize.w);

		ImGui::Image({ pTexture, intptr_t(pTexture.get()), 0 }, ImVec2(m_v4RenderViewSize.z, m_v4RenderViewSize.w));

		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(m_v4RenderViewSize.x, m_v4RenderViewSize.y, m_v4RenderViewSize.z, m_v4RenderViewSize.w);
		m_pGuizmoWidget->Render();
	}
    else
    {
     
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(0, 0), 0, ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
		ImGui::SetNextWindowBgAlpha(0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::Begin("MainView", NULL, ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar);

		m_v4RenderViewSize = Vector4(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

		ImGui::Image({ pTexture, intptr_t(pTexture.get()), 0 }, ImVec2(m_v4RenderViewSize.z, m_v4RenderViewSize.w));
		ImGui::End();
		ImGui::PopStyleVar(2);

		ImGuizmo::SetRect(m_v4RenderViewSize.x, m_v4RenderViewSize.y, m_v4RenderViewSize.z, m_v4RenderViewSize.w);
		m_pGuizmoWidget->Render();
	}

	GetMainEditor()->GetSceneTexture()->SetRect(Vector2(m_v4RenderViewSize.x, m_v4RenderViewSize.y), Vector2(m_v4RenderViewSize.z, m_v4RenderViewSize.w));
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

void MainView::Input(MInputEvent* pEvent)
{

}
