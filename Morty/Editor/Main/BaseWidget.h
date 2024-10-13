#pragma once

#include "Utility/MGlobal.h"
#include "imgui.h"

namespace morty
{

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

    virtual void             Render() = 0;

    virtual void             Initialize(MainEditor* pMainEditor);

    virtual void             Release() = 0;

    virtual void             SaveConfig(IniConfig* pConfig);

    virtual void             LoadConfig(IniConfig* pConfig);

    virtual void             Input(MInputEvent* pEvent) { MORTY_UNUSED(pEvent); }

    virtual ImGuiWindowFlags GetWindowFlags() { return ImGuiWindowFlags_NoCollapse; }

    MEngine*                 GetEngine() const;

    MScene*                  GetScene() const;

    MViewport*               GetViewport() const;

    MainEditor*              GetMainEditor() const { return m_mainEditor; }

    morty::MString           GetName() const { return m_strViewName; }

    bool                     GetVisible() const { return m_visiable; }

    void                     SetVisible(bool bVisible) { m_visiable = bVisible; }

    bool                     GetRenderInHidden() const { return m_renderInHidden; }

    void                     AddWidget(BaseWidget* pWidget);

protected:
    MainEditor*              m_mainEditor     = nullptr;
    morty::MString           m_strViewName    = "";
    bool                     m_visiable       = false;
    bool                     m_renderInHidden = false;

    std::vector<BaseWidget*> m_children;
};

}// namespace morty