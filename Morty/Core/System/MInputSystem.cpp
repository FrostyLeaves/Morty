#include "System/MInputSystem.h"

#include "Input/MInputEvent.h"
#include "Utility/MGlobal.h"

MORTY_CLASS_IMPLEMENT(MInputSystem, MISystem)

MInputSystem::MInputSystem()
	: MISystem()
	, m_v2MouseAddition()
{

}

MInputSystem::~MInputSystem()
{

}

void MInputSystem::Input(MInputEvent* pEvent)
{
	if (MMouseInputEvent* pMouseInputEvent = dynamic_cast<MMouseInputEvent*>(pEvent))
	{
		m_v2MouseAddition = pMouseInputEvent->GetMouseAddition();
	}
}

bool MInputSystem::IsKeyDown(const int& nKey)
{
	return MKeyBoardInputEvent::IsKeyDown(nKey);
}

bool MInputSystem::IsMouseButtonDown(const MMouseInputEvent::MEMouseDownButton& eButton)
{
	return MMouseInputEvent::IsButtonDown(eButton);
}

void MInputSystem::EngineTick(const float& fDelta)
{
	MORTY_UNUSED(fDelta);
	
	m_v2MouseAddition.x = 0.0f;
	m_v2MouseAddition.y = 0.0f;
}
