#include "SelectionEntityManager.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"

using namespace morty;

void SelectionEntityManager::SetSelectedEntity(MEntity* pEntity)
{
    if (pEntity)
    {
        m_pScene = pEntity->GetScene();
        m_selectedGuid = pEntity->GetID();
    }
    else
    {
        m_pScene = nullptr;
        m_selectedGuid = MGuid::invalid;
    }
}

MEntity* SelectionEntityManager::GetSelectedEntity() const
{
    if (!m_pScene)
    {
        return nullptr;
    }

    return m_pScene->GetEntity(m_selectedGuid);
}
