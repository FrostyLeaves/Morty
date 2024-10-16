/**
 * @File         MMoveControllerSystem
 * 
 * @Created      2021-08-20 11:45:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

#include "Math/Vector.h"

namespace morty
{

class MMoveControllerComponent;
class MORTY_API MMoveControllerSystem : public MISystem
{
    MORTY_CLASS(MMoveControllerSystem)
public:
    MMoveControllerSystem();

    virtual ~MMoveControllerSystem();

public:
    void SceneTick(MScene* pScene, const float& fDelta) override;

public:
    void UpdateTransform(MMoveControllerComponent* pComponent, const float& fDelta, const Vector2& v2MouseAddi);
};

}// namespace morty