#include "System/MMoveControllerSystem.h"

#include "Scene/MScene.h"
#include "Input/MInputEvent.h"

#include "Component/MSceneComponent.h"
#include "Component/MMoveControllerComponent.h"

#include "System/MInputSystem.h"

MORTY_CLASS_IMPLEMENT(MMoveControllerSystem, MISystem)

MMoveControllerSystem::MMoveControllerSystem()
	: MISystem()
{

}

MMoveControllerSystem::~MMoveControllerSystem()
{

}

void MMoveControllerSystem::SceneTick(MScene* pScene, const float& fDelta)
{
	MInputSystem* pInputSystem = GetEngine()->FindSystem<MInputSystem>();
	if (!pInputSystem)
		return;

	MComponentGroup<MMoveControllerComponent>* pComponents = pScene->FindComponents<MMoveControllerComponent>();
	if (!pComponents)
		return;

	for (MMoveControllerComponent& comp : pComponents->m_vComponents)
	{
		if (comp.IsValid())
		{
			UpdateTransform(&comp, fDelta, pInputSystem->GetMouseAddition());
		}
	}
}

void MMoveControllerSystem::UpdateTransform(MMoveControllerComponent* pComponent, const float& fDelta, const Vector2& v2MouseAddi)
{
	MEntity* pEntity = pComponent->GetEntity();
	if (!pEntity)
		return;

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return;

	const float speed = pComponent->GetMaxSpeed();
	Vector3 v3MoveSpeed = pComponent->GetMoveSpeed();

	if (true == MKeyBoardInputEvent::IsKeyDown('w'))
	{
		v3MoveSpeed += pSceneComponent->GetForward() * speed * fDelta;
	}
	if (true == MKeyBoardInputEvent::IsKeyDown('s'))
	{
		v3MoveSpeed += pSceneComponent->GetForward() * -speed * fDelta;
	}
	if (true == MKeyBoardInputEvent::IsKeyDown('a'))
	{
		v3MoveSpeed += pSceneComponent->GetRight() * -speed * fDelta;
	}
	if (true == MKeyBoardInputEvent::IsKeyDown('d'))
	{
		v3MoveSpeed += pSceneComponent->GetRight() * speed * fDelta;
	}
	if (true == MKeyBoardInputEvent::IsKeyDown('q'))
	{
		v3MoveSpeed += Vector3(0, 1, 0) * -speed * fDelta;
	}
	if (true == MKeyBoardInputEvent::IsKeyDown('e'))
	{
		v3MoveSpeed += Vector3(0, 1, 0) * speed * fDelta;
	}
	else if (MMouseInputEvent::IsButtonDown(MMouseInputEvent::MEMouseDownButton::RightButton) && (v2MouseAddi.x != 0 || v2MouseAddi.y != 0))
	{

		Vector3 up = Vector3(0, 1, 0);

		Vector3 right = pSceneComponent->GetRight();
		pSceneComponent->SetRotation(pSceneComponent->GetRotation() * Quaternion(up, v2MouseAddi.x * 0.25f) * Quaternion(right, v2MouseAddi.y * 0.25f));
	}

	float fLength = v3MoveSpeed.Length();
	if (fLength > speed)
	{
		v3MoveSpeed.Normalize();
		v3MoveSpeed = v3MoveSpeed * speed;
	}

	pComponent->SetMoveSpeed(v3MoveSpeed * 0.8f);
	pSceneComponent->SetPosition(pSceneComponent->GetPosition() + v3MoveSpeed);
}
