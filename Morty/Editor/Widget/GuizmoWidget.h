#pragma once

#include "Main/BaseWidget.h"
#include "imgui.h"

#include "ImGuizmo.h"

namespace morty
{

class GuizmoWidget : public BaseWidget
{
public:
    GuizmoWidget();

    ~GuizmoWidget() = default;

    void Render() override;

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;


private:
    ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      m_gizmoMode      = ImGuizmo::WORLD;
};

}// namespace morty