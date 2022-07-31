#ifndef _MMOVE_CONTROLLER_COMPONENT_H_
#define _MMOVE_CONTROLLER_COMPONENT_H_

#include "Component/MComponent.h"

class MNode;
class MMoveControllerComponent : public MComponent
{
public:
	MORTY_CLASS(MMoveControllerComponent)
public:
	MMoveControllerComponent();

	void SetMaxSpeed(const float& fSpeed) { m_fMaxSpeed = fSpeed; }
	float GetMaxSpeed() const { return m_fMaxSpeed; }

	void SetMoveSpeed(const Vector3& v3Speed) { m_v3MoveSpeed = v3Speed; }
	Vector3 GetMoveSpeed() const { return m_v3MoveSpeed; }

private:

	float m_fMaxSpeed;
	Vector3 m_v3MoveSpeed;
};

#endif