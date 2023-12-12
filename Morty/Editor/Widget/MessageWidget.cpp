#include "MessageWidget.h"

#include "imgui.h"
#include "Engine/MEngine.h"
#include "Utility/RenderMessageManager.h"

MessageWidget::MessageWidget()
	: BaseWidget()
{
	m_strViewName = "Message";
}

void MessageWidget::Render()
{
	const float DISTANCE = 10.0f;
	static int corner = -1;
	if (corner != -1)
	{
		ImGuiIO& io = ImGui::GetIO();

		ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	}
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (ImGui::Begin("Message", &m_bVisiable, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		int nCurrentFps = (int)round(GetEngine()->GetFPS() / 5) * 5;
		ImGui::Text("FPS: %d", nCurrentFps);

		ImGui::Text("Draw Call Count: %zu", RenderMessageManager::GetInstance()->nDrawCallCount);


		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (ImGui::MenuItem("Close")) m_bVisiable = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void MessageWidget::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);
}

void MessageWidget::Release()
{

}
