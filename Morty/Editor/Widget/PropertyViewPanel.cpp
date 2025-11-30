#include "PropertyViewPanel.h"
#include <imgui_internal.h>

#include "imgui.h"

#include "Basic/MViewport.h"
#include "MaterialPropertyRenderer.h"
#include "Object/MObject.h"
#include "Resource/MMaterialResource.h"
#include "Utility/SelectionContext.h"
#include "Widget/Property/MComponentPropertyList.h"

using namespace morty;

PropertyViewPanel::PropertyViewPanel(int panelID)
    : BaseWidget()
    , m_panelID(panelID)
    , m_propertyList()
{
    m_strViewName = MString("Property ") + std::to_string(panelID);
}

PropertyViewPanel::~PropertyViewPanel()
{
    for (auto* pPropertyBase: m_propertyList)
    {
        if (pPropertyBase) delete pPropertyBase;
    }

    m_propertyList.clear();
}

void PropertyViewPanel::Render()
{
    RenderLockButton();

    // Render path header
    RenderPathHeader();

    ImGui::Separator();

    if (m_currentSelection.type == SelectionType::Entity && m_currentSelection.entity)
    {
        RenderEntityProperties(m_currentSelection.entity);
    }
    else if (m_currentSelection.type == SelectionType::Resource && m_currentSelection.resource)
    {
        if (auto material = MTypeClass::DynamicCast<MMaterialResource>(m_currentSelection.resource))
        {
            RenderMaterialProperties(material);
        }
    }
}

void PropertyViewPanel::Initialize(MainEditor* pMainEditor)
{
    BaseWidget::Initialize(pMainEditor);

    m_materialRenderer = std::make_unique<MaterialPropertyRenderer>();
    m_materialRenderer->Initialize(pMainEditor);

    // Register for selection changes
    SelectionManager::GetInstance()->RegisterListener(this, [this](const Selection& selection) {
        OnSelectionChanged(selection);
    });
}

void PropertyViewPanel::Release()
{
    SelectionManager::GetInstance()->UnregisterListener(this);

    if (m_materialRenderer)
    {
        m_materialRenderer->Release(GetMainEditor());
        m_materialRenderer.reset();
    }
}

void PropertyViewPanel::Input(MInputEvent* pEvent)
{
    if (m_materialRenderer && m_materialRenderer->GetSceneViewer())
    {
        m_materialRenderer->GetSceneViewer()->GetViewport()->Input(pEvent);
    }
}

ImGuiWindowFlags PropertyViewPanel::GetWindowFlags() { return ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar; }

void             PropertyViewPanel::OnSelectionChanged(const Selection& selection)
{
    if (m_locked) return;

    m_currentSelection = selection;
    m_entity           = nullptr;

    if (selection.type == SelectionType::Entity)
    {
        m_entity = selection.entity;
        UpdatePropertyList(m_entity);
    }
}

void PropertyViewPanel::RenderEntityProperties(MEntity* pEntity)
{
    if (!pEntity) return;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    ImGuiContext& g      = *GImGui;
    auto          window = g.CurrentWindow;
    MORTY_ASSERT(window);

    ImGui::Columns(2);
    ImGui::Separator();

    if (m_entity)
    {
        for (auto* pPropertyBase: m_propertyList)
        {
            if (pPropertyBase) { pPropertyBase->EditEntity(m_mainEditor, m_entity); }
        }
    }

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::PopStyleVar();
}

void PropertyViewPanel::RenderMaterialProperties(std::shared_ptr<MMaterialResource> material)
{
    if (m_materialRenderer) { m_materialRenderer->RenderMaterialProperties(material); }
}

void PropertyViewPanel::UpdatePropertyList(MEntity* pEntity)
{
    for (auto* pPropertyBase: m_propertyList) { delete pPropertyBase; }
    m_propertyList.clear();

    auto vComponents = pEntity->GetComponents();
    for (MComponent* component: vComponents)
    {
        auto findResult = MComponentPropertyList::EditFactory.find(component->GetTypeName());
        if (findResult != MComponentPropertyList::EditFactory.end()) { m_propertyList.push_back(findResult->second()); }
    }
}

void PropertyViewPanel::RenderLockButton()
{
    if (ImGui::BeginMenuBar())
    {
        const char* lockIcon = m_locked ? "Locked" : "Unlocked";
        if (ImGui::Button(lockIcon)) { m_locked = !m_locked; }

        ImGui::EndMenuBar();
    }
}

void PropertyViewPanel::RenderPathHeader()
{
    MString path;
    bool    hasPath = false;

    if (m_currentSelection.type == SelectionType::Entity && m_currentSelection.entity)
    {
        // For entities, show their name as path
        path    = "Entity: " + m_currentSelection.entity->GetName();
        hasPath = true;
    }
    else if (m_currentSelection.type == SelectionType::Resource && m_currentSelection.resource)
    {
        // For resources, show their resource path
        path    = "Resource: " + m_currentSelection.resource->GetResourcePath();
        hasPath = true;
    }

    if (hasPath)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));

        if (ImGui::Button(path.c_str(), ImVec2(-1, 0))) { OnPathClicked(); }

        if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Click to select in tree/resource view"); }

        ImGui::PopStyleColor(3);
    }
}

void PropertyViewPanel::OnPathClicked()
{
    if (m_currentSelection.type == SelectionType::Entity && m_currentSelection.entity)
    {
        // Re-select the entity to ensure it's highlighted in the tree view
        SelectionContext::GetInstance()->SetSelectedEntity(m_currentSelection.entity);
        SelectionManager::GetInstance()->BroadcastSelection(Selection(m_currentSelection.entity));
    }
    else if (m_currentSelection.type == SelectionType::Resource && m_currentSelection.resource)
    {
        // Re-select the resource to ensure it's highlighted in the resource view
        SelectionContext::GetInstance()->SetSelectedResource(m_currentSelection.resource);
        SelectionManager::GetInstance()->BroadcastSelection(Selection(m_currentSelection.resource));
    }
}
