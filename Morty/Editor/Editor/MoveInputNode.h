#ifndef _EDITOR_CAMERA_H_
#define _EDITOR_CAMERA_H_

#include "MInputNode.h"

class M3DNode;
class MoveInputNode : public MInputNode
{
public:
	MoveInputNode();

	void SetMoveNode(M3DNode* pNode);

	virtual void OnTick(const float& fDelta);

	std::map<unsigned int, bool> m_tKeyBoardDown;
	
	virtual void OnCreated() override;

	float m_fMaxSpeed;
	Vector2 m_v2MouseAddi;
	Vector3 m_v3MoveSpeed;


	M3DNode* m_pMoveNode;
};

#endif