#pragma once

#include "Main/BaseWidget.h"
#include "Render/SceneTexture.h"

#include "Resource/MResource.h"
#include "Property/PropertyBase.h"

MORTY_SPACE_BEGIN

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
	GuizmoWidget* m_pGuizmoWidget = nullptr;
};

MORTY_SPACE_END