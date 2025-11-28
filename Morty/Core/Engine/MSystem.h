#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"

namespace morty
{

class MScene;
class MEngine;
class MORTY_API MISystem : public MTypeClass
{
public:
    MORTY_INTERFACE(MISystem)

public:
    MISystem();

    virtual ~MISystem();

    virtual void Initialize() {};

    virtual void Release() {};


    virtual void EngineTick(const float& fDelta) { MORTY_UNUSED(fDelta); }

    virtual void SceneTick(MScene* scene, const float& fDelta) { MORTY_UNUSED(scene, fDelta); }


    void         SetEngine(MEngine* engine);

    MEngine*     GetEngine();

private:
    MEngine* m_engine;
};

}// namespace morty