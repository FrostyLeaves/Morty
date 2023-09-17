#pragma  once

#include "SingletonInstance.h"
#include "Scene/MGuid.h"

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