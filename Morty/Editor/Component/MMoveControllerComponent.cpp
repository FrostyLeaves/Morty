#include "MMoveControllerComponent.h"

#include "MEntity.h"
#include "MEngine.h"
#include "MObject.h"
#include "MInputEvent.h"

#include "MInputComponent.h"
#include "MSceneComponent.h"

MORTY_CLASS_IMPLEMENT(MMoveControllerComponent, MComponent)

MMoveControllerComponent::MMoveControllerComponent()
	: MComponent()
	, m_fMaxSpeed(6.0f)
	, m_v3MoveSpeed(0.0f, 0.0f, 0.0f)
{

}
