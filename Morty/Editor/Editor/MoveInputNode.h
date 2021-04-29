#ifndef _EDITOR_CAMERA_H_
#define _EDITOR_CAMERA_H_

#include "MInputComponent.h"

class MNode;
class MoveInputComponent : public MInputComponent
{
public:
	M_OBJECT(MoveInputComponent)
public:
	MoveInputComponent();

	virtual void Tick(const float& fDelta) override;

	std::map<unsigned int, bool> m_tKeyBoardDown;
	
	virtual void Initialize() override;

	float m_fMaxSpeed;
	Vector2 m_v2MouseAddi;
	Vector3 m_v3MoveSpeed;
};

#endif