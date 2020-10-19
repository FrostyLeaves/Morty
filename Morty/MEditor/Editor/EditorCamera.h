#ifndef _EDITOR_CAMERA_H_
#define _EDITOR_CAMERA_H_

#include "MCamera.h"

class EditorCamera : public MCamera
{
public:
	EditorCamera();

	virtual void OnTick(const float& fDelta);

	std::map<unsigned int, bool> m_tKeyBoardDown;
	
	virtual void OnCreated() override;

	float m_fMaxSpeed;
	Vector2 m_v2MouseAddi;
	Vector3 m_v3MoveSpeed;
};

#endif