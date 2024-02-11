#pragma once

#include "Component/MComponent.h"
#include "Math/Vector.h"

MORTY_SPACE_BEGIN

class MNode;
class MMoveControllerComponent : public morty::MComponent
{
public:
	MORTY_CLASS(MMoveControllerComponent)
public:
	MMoveControllerComponent() = default;

	void SetMaxSpeed(const float& fSpeed) { m_fMaxSpeed = fSpeed; }
	float GetMaxSpeed() const { return m_fMaxSpeed; }

	void SetMoveSpeed(const morty::Vector3& v3Speed) { m_v3MoveSpeed = v3Speed; }
	morty::Vector3 GetMoveSpeed() const { return m_v3MoveSpeed; }

private:

	float m_fMaxSpeed = 6.0f;
	morty::Vector3 m_v3MoveSpeed = {};
};

MORTY_SPACE_END