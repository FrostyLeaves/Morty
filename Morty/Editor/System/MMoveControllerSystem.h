/**
 * @File         MMoveControllerSystem
 * 
 * @Created      2021-08-20 11:45:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMOVECONTROLLERSYSTEM_H_
#define _M_MMOVECONTROLLERSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

#include "Math/Vector.h"

class MMoveControllerComponent;
class MORTY_API MMoveControllerSystem : public MISystem
{
    MORTY_CLASS(MMoveControllerSystem)
public:
    MMoveControllerSystem();
    virtual ~MMoveControllerSystem();

public:

    virtual void SceneTick(MScene* pScene, const float& fDelta);

public:

    void UpdateTransform(MMoveControllerComponent* pComponent, const float& fDelta, const Vector2& v2MouseAddi);
};


#endif
