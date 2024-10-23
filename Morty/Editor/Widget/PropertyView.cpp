#include "PropertyView.h"
#include <imgui_internal.h>

#include "imgui.h"

#include "Object/MObject.h"

#include "Property/PropertyMCameraComponent.h"
#include "Property/PropertyMDirectionalLightComponent.h"
#include "Property/PropertyMModelComponent.h"
#include "Property/PropertyMPointLight.h"
#include "Property/PropertyMRenderMeshComponent.h"
#include "Property/PropertyMSceneComponent.h"
#include "Property/PropertyMSpotLight.h"
#include "Utility/SelectionEntityManager.h"

using namespace morty;

#define REGISTER_PROPERTY(CLASS_NAME)                                                                                  \
    m_createPropertyFactory[MStringId(#CLASS_NAME)] = []() { return new Property##CLASS_NAME(); };

PropertyView::PropertyView()
    : BaseWidget()
    , m_propertyList()
{
    m_strViewName = "Property";

    REGISTER_PROPERTY(MSceneComponent);
    REGISTER_PROPERTY(MCameraComponent);
    REGISTER_PROPERTY(MSpotLightComponent);
    REGISTER_PROPERTY(MPointLightComponent);
    REGISTER_PROPERTY(MDirectionalLightComponent);
    REGISTER_PROPERTY(MModelComponent);
    REGISTER_PROPERTY(MRenderMeshComponent);
}

PropertyView::~PropertyView()
{
    for (PropertyBase* pPropertyBase: m_propertyList)
    {
        if (pPropertyBase) delete pPropertyBase;
    }

    m_propertyList.clear();
}

void PropertyView::Render()
{
    MEntity* pEntity = SelectionEntityManager::GetInstance()->GetSelectedEntity();
    if (nullptr == pEntity) return;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    ImGuiContext& g      = *GImGui;
    auto          window = g.CurrentWindow;
    MORTY_ASSERT(window);

    ImGui::Columns(2);
    ImGui::Separator();

    EditEntity(pEntity);

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
}

void PropertyView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void PropertyView::Release() {}

void PropertyView::EditEntity(MEntity* pEntity)
{
    if (m_entity != pEntity)
    {
        m_entity = pEntity;
        UpdatePropertyList(pEntity);
    }

    if (m_entity)
    {
        for (PropertyBase* pPropertyBase: m_propertyList)
        {
            if (pPropertyBase) { pPropertyBase->EditEntity(m_entity); }
        }
    }
}

void PropertyView::UpdatePropertyList(MEntity* pEntity)
{
    for (PropertyBase* pPropertyBase: m_propertyList)
    {
        if (pPropertyBase) delete pPropertyBase;
    }
    m_propertyList.clear();

    auto vComponents = pEntity->GetComponents();
    for (MComponent* pComponent: vComponents)
    {
        if (auto func = m_createPropertyFactory[pComponent->GetTypeName()]) { m_propertyList.push_back(func()); }
    }
}
