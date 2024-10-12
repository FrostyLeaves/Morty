#include "Input/MInputEvent.h"

using namespace morty;

uint32_t MMouseInputEvent::s_unMouseDownButton = 0;
Vector2  MMouseInputEvent::s_v2MousePosition(-1, -1);

bool     MKeyBoardInputEvent::s_vKeyDownMap[256] = {0};

MInputEvent::MInputEvent()
    : m_isAccepted(false)
{}

MInputEvent::~MInputEvent() {}

MMouseInputEvent::MMouseInputEvent(
        const MEMouseDownButton& eMouseDownButton,
        const MEMouseInputType&  eInputType
)
    : m_eventButton(eMouseDownButton)
{
    if (MEMouseInputType::ButtonDown == eInputType)
    {
        s_unMouseDownButton |= eMouseDownButton;
    }
    else if (MEMouseInputType::ButtonUp == eInputType)
    {
        s_unMouseDownButton &= ~eMouseDownButton;
    }

    m_inputType = eInputType;

    m_mousePositionAddition = Vector2(0, 0);
}

MMouseInputEvent::MMouseInputEvent(
        const Vector2& v2MousePosition,
        const Vector2& v2MousePositionAddition
)
    : m_eventButton(MEMouseDownButton::NoneButton)
    , m_inputType(MEMouseInputType::MouseMove)
{
    m_mousePositionAddition = v2MousePositionAddition;
    s_v2MousePosition       = v2MousePosition;
}

bool MMouseInputEvent::IsButtonDown(const MEMouseDownButton& eButton)
{
    return s_unMouseDownButton & eButton;
}

MKeyBoardInputEvent::MKeyBoardInputEvent(
        const uint32_t&   unKeyIndex,
        const MEKeyState& eInputType
)
    : m_unKeyIndex(0)
{
    if (unKeyIndex < 256)
    {
        s_vKeyDownMap[unKeyIndex] = MEKeyState::DOWN == eInputType;

        m_unKeyIndex = unKeyIndex;
    }

    m_inputType = eInputType;
}

bool MKeyBoardInputEvent::IsKeyDown(const int& nKey)
{
    if (nKey < 256) { return s_vKeyDownMap[nKey]; }

    return false;
}
