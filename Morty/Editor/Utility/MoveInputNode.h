#ifndef _MOVE_INPUT_COMPONENT_H_
#define _MOVE_INPUT_COMPONENT_H_

#include "MInputComponent.h"

class MNode;
class MoveInputComponent : public MInputComponent
{
public:
	MORTY_CLASS(MoveInputComponent)
public:
	MoveInputComponent();

	void Tick(const float& fDelta);

	std::map<unsigned int, bool> m_tKeyBoardDown;
	
	virtual void Initialize() override;

	float m_fMaxSpeed;
	Vector2 m_v2MouseAddi;
	Vector3 m_v3MoveSpeed;
};

#endif