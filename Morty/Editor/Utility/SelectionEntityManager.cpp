#include "SelectionEntityManager.h"

#include "Scene/MEntity.h"
#include "Scene/MScene.h"

using namespace morty;

void SelectionEntityManager::SetSelectedEntity(MEntity* pEntity)
{
    if (pEntity)
    {
        m_scene        = pEntity->GetScene();
        m_selectedGuid = pEntity->GetID();
    }
    else
    {
        m_scene        = nullptr;
        m_selectedGuid = MGuid::invalid;
    }
}

MEntity* SelectionEntityManager::GetSelectedEntity() const
{
    if (!m_scene) { return nullptr; }

    return m_scene->GetEntity(m_selectedGuid);
}
