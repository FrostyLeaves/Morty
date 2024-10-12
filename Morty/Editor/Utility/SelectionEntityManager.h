#pragma once

#include "Scene/MGuid.h"
#include "SingletonInstance.h"

namespace morty
{

class MScene;
class MEntity;
class SelectionEntityManager : public SingletonInstance<SelectionEntityManager>
{
public:
    void     SetSelectedEntity(MEntity* pEntity);

    MEntity* GetSelectedEntity() const;

private:
    MScene* m_scene = nullptr;
    MGuid   m_selectedGuid;
};

}// namespace morty