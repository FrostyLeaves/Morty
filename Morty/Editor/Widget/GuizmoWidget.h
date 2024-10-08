#pragma once

#include "Main/BaseWidget.h"

#include "imgui.h"
#include "ImGuizmo.h"

MORTY_SPACE_BEGIN

class GuizmoWidget : public BaseWidget
{
public:
	GuizmoWidget();
    ~GuizmoWidget() = default;

	void Render() override;
	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;


private:

	ImGuizmo::OPERATION m_eGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_eGizmoMode = ImGuizmo::WORLD;
};

MORTY_SPACE_END