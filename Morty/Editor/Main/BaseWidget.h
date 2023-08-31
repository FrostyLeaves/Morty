#pragma once

#include "Utility/MGlobal.h"

class MViewport;
class MScene;
class MEngine;
class MainEditor;
class MInputEvent;
class BaseWidget
{
public:
	BaseWidget() {}
	virtual ~BaseWidget() {}

	virtual void Render() = 0;

	virtual void Initialize(MainEditor* pMainEditor);
	virtual void Release() = 0;

	virtual void Input(MInputEvent* pEvent) = 0;

	MEngine* GetEngine() const;
	MScene* GetScene() const;
	MViewport* GetViewport() const;
	MainEditor* GetMainEditor() const { return m_pMainEditor; }
	MString GetName() const { return m_strViewName; }
	bool GetVisible() const { return m_bVisiable; }
	void SetVisible(bool bVisible) { m_bVisiable = bVisible; }
	bool GetRenderInHidden() const { return m_bRenderInHidden; }

	void AddWidget(BaseWidget* pWidget);

protected:
	MainEditor* m_pMainEditor = nullptr;
	MString m_strViewName = "";
	bool m_bVisiable = false;
	bool m_bRenderInHidden = false;

	std::vector<BaseWidget*> m_vChildren;
};