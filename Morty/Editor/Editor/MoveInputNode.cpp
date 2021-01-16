#include "MoveInputNode.h"

#include "MEngine.h"
#include "MObject.h"
#include "M3DNode.h"
#include "MInputNode.h"
#include "MInputManager.h"

MoveInputNode::MoveInputNode()
	: MInputNode()
	, m_fMaxSpeed(6.0f)
	, m_v2MouseAddi(0.0f, 0.0f)
	, m_v3MoveSpeed(0.0f, 0.0f, 0.0f)
	, m_pMoveNode(nullptr)
{

}

void MoveInputNode::SetMoveNode(M3DNode* pNode)
{
	m_pMoveNode = pNode;
}

void MoveInputNode::OnTick(const float& fDelta)
{
	if (!m_pMoveNode)
		return;

	const float speed = m_fMaxSpeed;

	if (true == m_tKeyBoardDown['W'])
	{
		m_v3MoveSpeed += m_pMoveNode->GetForward() * speed * fDelta;
	}
	if (true == m_tKeyBoardDown['S'])
	{
		m_v3MoveSpeed += m_pMoveNode->GetForward() * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['A'])
	{
		m_v3MoveSpeed += m_pMoveNode->GetRight() * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['D'])
	{
		m_v3MoveSpeed += m_pMoveNode->GetRight() * speed * fDelta;
	}
	if (true == m_tKeyBoardDown['Q'])
	{
		m_v3MoveSpeed += Vector3(0, 1, 0) * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['E'])
	{
		m_v3MoveSpeed += Vector3(0, 1, 0) * speed * fDelta;
	}
	else if (m_tKeyBoardDown[MMouseInputEvent::MEMouseDownButton::RightButton] && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
	{

		Vector3 up = Vector3(0, 1, 0);

		Vector3 right = m_pMoveNode->GetRight();
		m_pMoveNode->SetRotation(m_pMoveNode->GetRotation() * Quaternion(up, m_v2MouseAddi.x * 0.25f) * Quaternion(right, m_v2MouseAddi.y * 0.25f));

		m_v2MouseAddi = Vector2(0, 0);
	}

	float fLength = m_v3MoveSpeed.Length();
	if (fLength > speed)
	{
		m_v3MoveSpeed.Normalize();
		m_v3MoveSpeed = m_v3MoveSpeed * speed;
	}

	m_pMoveNode->SetPosition(m_pMoveNode->GetPosition() + m_v3MoveSpeed);
	m_v3MoveSpeed = m_v3MoveSpeed * 0.8f;
}

void MoveInputNode::OnCreated()
{
	SetInputCallback([this](MInputEvent* pEvent, MViewport* pViewport) {

		if (MKeyBoardInputEvent* pKeyInput = dynamic_cast<MKeyBoardInputEvent*>(pEvent))
		{
			m_tKeyBoardDown[pKeyInput->GetKey()] = pKeyInput->GetType() == MEKeyState::DOWN;

		}
		else if (MMouseInputEvent* pMouseInput = dynamic_cast<MMouseInputEvent*>(pEvent))
		{
			if (pMouseInput->GetType() == MMouseInputEvent::MouseMove)
			{
				m_v2MouseAddi = pMouseInput->GetMouseAddition();
			}
			else
			{
				m_tKeyBoardDown[(unsigned int)pMouseInput->GetButton()] = pMouseInput->GetType() == MMouseInputEvent::ButtonDown;
			}
		}

		return false;
	});
}

