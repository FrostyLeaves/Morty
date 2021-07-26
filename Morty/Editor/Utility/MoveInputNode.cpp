#include "MoveInputNode.h"

#include "MEntity.h"
#include "MEngine.h"
#include "MObject.h"
#include "MInputManager.h"

#include "MSceneComponent.h"

MORTY_CLASS_IMPLEMENT(MoveInputComponent, MInputComponent)

MoveInputComponent::MoveInputComponent()
	: MInputComponent()
	, m_fMaxSpeed(6.0f)
	, m_v2MouseAddi(0.0f, 0.0f)
	, m_v3MoveSpeed(0.0f, 0.0f, 0.0f)
{

}

void MoveInputComponent::Tick(const float& fDelta)
{
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return;

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return;

	const float speed = m_fMaxSpeed;

	if (true == m_tKeyBoardDown['w'])
	{
		m_v3MoveSpeed += pSceneComponent->GetForward() * speed * fDelta;
	}
	if (true == m_tKeyBoardDown['s'])
	{
		m_v3MoveSpeed += pSceneComponent->GetForward() * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['a'])
	{
		m_v3MoveSpeed += pSceneComponent->GetRight() * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['d'])
	{
		m_v3MoveSpeed += pSceneComponent->GetRight() * speed * fDelta;
	}
	if (true == m_tKeyBoardDown['q'])
	{
		m_v3MoveSpeed += Vector3(0, 1, 0) * -speed * fDelta;
	}
	if (true == m_tKeyBoardDown['e'])
	{
		m_v3MoveSpeed += Vector3(0, 1, 0) * speed * fDelta;
	}
	else if (m_tKeyBoardDown[MMouseInputEvent::MEMouseDownButton::RightButton] && (m_v2MouseAddi.x != 0 || m_v2MouseAddi.y != 0))
	{

		Vector3 up = Vector3(0, 1, 0);

		Vector3 right = pSceneComponent->GetRight();
		pSceneComponent->SetRotation(pSceneComponent->GetRotation() * Quaternion(up, m_v2MouseAddi.x * 0.25f) * Quaternion(right, m_v2MouseAddi.y * 0.25f));

		m_v2MouseAddi = Vector2(0, 0);
	}

	float fLength = m_v3MoveSpeed.Length();
	if (fLength > speed)
	{
		m_v3MoveSpeed.Normalize();
		m_v3MoveSpeed = m_v3MoveSpeed * speed;
	}

	pSceneComponent->SetPosition(pSceneComponent->GetPosition() + m_v3MoveSpeed);
	m_v3MoveSpeed = m_v3MoveSpeed * 0.8f;
}

void MoveInputComponent::Initialize()
{
	Super::Initialize();


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

