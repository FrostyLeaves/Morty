#pragma once

#include "Main/BaseWidget.h"

namespace morty
{

class MType;
class ResourceView : public BaseWidget
{
public:
    ResourceView();

    ~ResourceView() override = default;

    void             Render() override;

    void             DrawMenu();

    void             ProcessDialog();

    void             Initialize(MainEditor* pMainEditor) override;

    void             Release() override;

    ImGuiWindowFlags GetWindowFlags() override { return BaseWidget::GetWindowFlags() | ImGuiWindowFlags_MenuBar; }

private:
    MString      m_createResourceDialogId;
    MString      m_createResourcePath;
    const MType* m_createResourceType = nullptr;
};

}// namespace morty