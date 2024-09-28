#pragma once

#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

class MViewport;
class MScene;
class MEngine;
class MainEditor;
class MInputEvent;
class IniConfig;

class BaseWidget
{
public:
	BaseWidget() {}
	virtual ~BaseWidget() {}

	virtual void Render() = 0;

	virtual void Initialize(MainEditor* pMainEditor);
	virtual void Release() = 0;

    virtual void SaveConfig(IniConfig* pConfig);
    virtual void LoadConfig(IniConfig* pConfig);

	virtual void Input(MInputEvent* pEvent) { MORTY_UNUSED(pEvent); }

	MEngine* GetEngine() const;
	MScene* GetScene() const;
	MViewport* GetViewport() const;
	MainEditor* GetMainEditor() const { return m_pMainEditor; }
	morty::MString GetName() const { return m_strViewName; }
	bool GetVisible() const { return m_bVisiable; }
	void SetVisible(bool bVisible) { m_bVisiable = bVisible; }
	bool GetRenderInHidden() const { return m_bRenderInHidden; }

	void AddWidget(BaseWidget* pWidget);

protected:
	MainEditor* m_pMainEditor = nullptr;
	morty::MString m_strViewName = "";
	bool m_bVisiable = false;
	bool m_bRenderInHidden = false;

	std::vector<BaseWidget*> m_vChildren;
};

MORTY_SPACE_END