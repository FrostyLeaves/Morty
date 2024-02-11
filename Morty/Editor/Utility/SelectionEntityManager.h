#pragma  once

#include "SingletonInstance.h"
#include "Scene/MGuid.h"

MORTY_SPACE_BEGIN

class MScene;
class MEntity;
class SelectionEntityManager : public SingletonInstance<SelectionEntityManager>
{
public:

    void SetSelectedEntity( MEntity* pEntity);
    MEntity* GetSelectedEntity() const;

private:
    MScene* m_pScene = nullptr;
    MGuid m_selectedGuid;
};

MORTY_SPACE_END