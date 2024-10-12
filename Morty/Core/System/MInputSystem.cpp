#include "System/MInputSystem.h"

#include "Utility/MGlobal.h"
#include "Input/MInputEvent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MInputSystem, MISystem)

MInputSystem::MInputSystem()
    : MISystem()
    , m_mouseAddition()
{}

MInputSystem::~MInputSystem() {}

void MInputSystem::Input(MInputEvent* pEvent)
{
    if (MMouseInputEvent* pMouseInputEvent = dynamic_cast<MMouseInputEvent*>(pEvent))
    {
        m_mouseAddition = pMouseInputEvent->GetMouseAddition();
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

    m_mouseAddition.x = 0.0f;
    m_mouseAddition.y = 0.0f;
}
