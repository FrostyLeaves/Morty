#pragma once

#include "Component/MComponent.h"
#include "Math/Vector.h"

namespace morty
{

class MNode;
class MMoveControllerComponent : public morty::MComponent
{
public:
    MORTY_CLASS(MMoveControllerComponent)
public:
    MMoveControllerComponent() = default;

    void           SetMaxSpeed(const float& fSpeed) { m_maxSpeed = fSpeed; }

    float          GetMaxSpeed() const { return m_maxSpeed; }

    void           SetMoveSpeed(const morty::Vector3& v3Speed) { m_moveSpeed = v3Speed; }

    morty::Vector3 GetMoveSpeed() const { return m_moveSpeed; }

private:
    float          m_maxSpeed  = 6.0f;
    morty::Vector3 m_moveSpeed = {};
};

}// namespace morty