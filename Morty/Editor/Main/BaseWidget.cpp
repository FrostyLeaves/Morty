#include "BaseWidget.h"
#include "MainEditor.h"
#include "Utility/IniConfig.h"

using namespace morty;

void BaseWidget::Initialize(MainEditor* pMainEditor) { m_mainEditor = pMainEditor; }

void BaseWidget::SaveConfig(IniConfig* pConfig)
{
    pConfig->SetValue<bool>(GetName().c_str(), "Visible", m_visiable);
}

void BaseWidget::LoadConfig(IniConfig* pConfig)
{
    m_visiable = pConfig->GetValue<bool>(GetName().c_str(), "Visible");
}

MEngine*   BaseWidget::GetEngine() const { return GetMainEditor()->GetEngine(); }


MScene*    BaseWidget::GetScene() const { return GetMainEditor()->GetScene(); }

MViewport* BaseWidget::GetViewport() const { return GetMainEditor()->GetViewport(); }

void       BaseWidget::AddWidget(BaseWidget* pWidget) { m_children.push_back(pWidget); }