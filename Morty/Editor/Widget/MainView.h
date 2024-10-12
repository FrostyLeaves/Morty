#pragma once

#include "Main/BaseWidget.h"
#include "Render/SceneViewer.h"

#include "Property/PropertyBase.h"
#include "Resource/MResource.h"

namespace morty
{

class GuizmoWidget;
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
    GuizmoWidget* m_guizmoWidget = nullptr;
};

}// namespace morty