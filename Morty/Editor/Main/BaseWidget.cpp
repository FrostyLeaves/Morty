#include "BaseWidget.h"
#include "MainEditor.h"

using namespace morty;

void BaseWidget::Initialize(MainEditor* pMainEditor)
{
    m_pMainEditor = pMainEditor;
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