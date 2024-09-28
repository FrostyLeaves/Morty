#include "BaseWidget.h"
#include "MainEditor.h"
#include "Utility/IniConfig.h"

using namespace morty;

void BaseWidget::Initialize(MainEditor* pMainEditor)
{
    m_pMainEditor = pMainEditor;
}

void BaseWidget::SaveConfig(IniConfig* pConfig)
{
    pConfig->SetValue<bool>(GetName().c_str(), "Visible", m_bVisiable);
}

void BaseWidget::LoadConfig(IniConfig* pConfig)
{
    m_bVisiable = pConfig->GetValue<bool>(GetName().c_str(), "Visible");
}

MEngine* BaseWidget::GetEngine() const
{
    return GetMainEditor()->GetEngine();
}


MScene* BaseWidget::GetScene() const
{
    return GetMainEditor()->GetScene();
}

MViewport* BaseWidget::GetViewport() const
{
    return GetMainEditor()->GetViewport();
}

void BaseWidget::AddWidget(BaseWidget* pWidget)
{
    m_vChildren.push_back(pWidget);
}