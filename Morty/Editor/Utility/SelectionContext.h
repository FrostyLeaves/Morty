#pragma once

#include "Resource/MResource.h"
#include "Scene/MGuid.h"
#include "SingletonInstance.h"
#include <memory>

namespace morty
{

class MScene;
class MEntity;

enum class SelectionContextType
{
    None,
    Entity,
    Resource
};

class SelectionContext : public SingletonInstance<SelectionContext>
{
public:
    void                       SetSelectedEntity(MEntity* pEntity);

    void                       SetSelectedResource(std::shared_ptr<MResource> pResource);

    void                       ClearSelection();

    MEntity*                   GetSelectedEntity() const;

    std::shared_ptr<MResource> GetSelectedResource() const;

    SelectionContextType       GetSelectionType() const { return m_selectionType; }

    bool                       IsEntitySelected() const { return m_selectionType == SelectionContextType::Entity; }

    bool                       IsResourceSelected() const { return m_selectionType == SelectionContextType::Resource; }

private:
    SelectionContextType       m_selectionType = SelectionContextType::None;

    // Entity selection
    MScene*                    m_scene        = nullptr;
    MGuid                      m_selectedGuid = MGuid::invalid;

    // Resource selection
    std::shared_ptr<MResource> m_selectedResource = nullptr;
};

}// namespace morty
