/**
 * @File         MModelSystem
 * 
 * @Created      2021-07-21 14:37:08
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMODELSYSTEM_H_
#define _M_MMODELSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

class MScene;
class MORTY_API MModelSystem : public MISystem
{
    MORTY_CLASS(MModelSystem)
public:
    MModelSystem();
    virtual ~MModelSystem();

public:

    virtual void SceneTick(MScene* pScene, const float& fDelta) override;

    void UpdateAnimation(MScene* pScene, const float& fDelta);

private:

};


#endif
