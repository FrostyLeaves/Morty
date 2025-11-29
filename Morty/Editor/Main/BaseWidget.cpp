#include "BaseWidget.h"
#include "MainEditor.h"
#include "Utility/IniConfig.h"
#include <algorithm>

using namespace morty;

void BaseWidget::Initialize(MainEditor* pMainEditor) { m_mainEditor = pMainEditor; }

void BaseWidget::SaveConfig(IniConfig* pConfig) { pConfig->SetValue<bool>(GetName().c_str(), "Visible", m_visiable); }

void BaseWidget::LoadConfig(IniConfig* pConfig)
{
    if (!pConfig) return;
    m_visiable = pConfig->GetValue<bool>(GetName().c_str(), "Visible");
}

MEngine*   BaseWidget::GetEngine() const { return GetMainEditor()->GetEngine(); }


MScene*    BaseWidget::GetScene() const { return GetMainEditor()->GetScene(); }

MViewport* BaseWidget::GetViewport() const { return GetMainEditor()->GetViewport(); }

void       BaseWidget::AddWidget(BaseWidget* pWidget) { m_children.push_back(pWidget); }

void       BaseWidget::RemoveWidget(BaseWidget* widget)
{
    auto it = std::find(m_children.begin(), m_children.end(), widget);
    if (it != m_children.end()) { m_children.erase(it); }
}