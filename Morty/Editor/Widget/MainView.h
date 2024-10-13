#pragma once

#include "Main/BaseWidget.h"
#include "Render/SceneViewer.h"

#include "Property/PropertyBase.h"
#include "Resource/MResource.h"

namespace morty
{

class GuizmoWidget;
class MessageWidget;
class MScene;
class MEntity;
class MEngine;
class MMaterialResource;
class MInputEvent;
class MainView : public BaseWidget
{
public:
    MainView();

    ~MainView() = default;

    void Initialize(MainEditor* pMainEditor) override;

    void Release() override;

    void Render() override;

private:
    GuizmoWidget*  m_guizmoWidget  = nullptr;
    MessageWidget* m_messageWidget = nullptr;
};

}// namespace morty