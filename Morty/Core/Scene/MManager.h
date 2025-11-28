#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"

namespace morty
{

class MScene;
class MEngine;
class MComponent;
class MORTY_API IManager : public MTypeClass
{
public:
    MORTY_INTERFACE(IManager)

public:
    IManager();

    virtual ~IManager();

    virtual void                   Initialize() {};

    virtual void                   Release() {};

    virtual std::set<const MType*> RegisterComponentType() const { return {}; }

    virtual void                   SceneTick(MScene* pScene, const float& fDelta) { MORTY_UNUSED(pScene, fDelta); }

    virtual void                   RegisterComponent(MComponent* component) { MORTY_UNUSED(component); }

    virtual void                   UnregisterComponent(MComponent* component) { MORTY_UNUSED(component); }

    void                           SetScene(MScene* pScene);

    MScene*                        GetScene();

    MEngine*                       GetEngine();

private:
    MScene* m_scene;
};

}// namespace morty