#include "Component/MMoveControllerComponent.h"

#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Input/MInputEvent.h"

#include "Component/MInputComponent.h"
#include "Component/MSceneComponent.h"

MORTY_CLASS_IMPLEMENT(MMoveControllerComponent, MComponent)

MMoveControllerComponent::MMoveControllerComponent()
	: MComponent()
	, m_fMaxSpeed(6.0f)
	, m_v3MoveSpeed(0.0f, 0.0f, 0.0f)
{

}
