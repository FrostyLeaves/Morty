#include "SelectionContext.h"

#include "Scene/MEntity.h"
#include "Scene/MScene.h"

using namespace morty;

void SelectionContext::SetSelectedEntity(MEntity* pEntity)
{
    if (pEntity)
    {
        m_selectionType    = SelectionContextType::Entity;
        m_scene            = pEntity->GetScene();
        m_selectedGuid     = pEntity->GetID();
        m_selectedResource = nullptr;
    }
    else { ClearSelection(); }
}

void SelectionContext::SetSelectedResource(std::shared_ptr<MResource> pResource)
{
    if (pResource)
    {
        m_selectionType    = SelectionContextType::Resource;
        m_selectedResource = pResource;
        m_scene            = nullptr;
        m_selectedGuid     = MGuid::invalid;
    }
    else { ClearSelection(); }
}

void SelectionContext::ClearSelection()
{
    m_selectionType    = SelectionContextType::None;
    m_scene            = nullptr;
    m_selectedGuid     = MGuid::invalid;
    m_selectedResource = nullptr;
}

MEntity* SelectionContext::GetSelectedEntity() const
{
    if (m_selectionType != SelectionContextType::Entity || !m_scene) { return nullptr; }

    return m_scene->GetEntity(m_selectedGuid);
}

std::shared_ptr<MResource> SelectionContext::GetSelectedResource() const
{
    if (m_selectionType != SelectionContextType::Resource) { return nullptr; }

    return m_selectedResource;
}
