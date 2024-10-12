/**
 * @File         MInputManager
 * 
 * @Created      2019-09-03 18:22:20
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"

namespace morty
{

enum class MEKeyState
{
    DOWN = 1,
    UP   = 2,
};

class MViewport;
class MORTY_API MInputEvent
{
public:
    MInputEvent();

    virtual ~MInputEvent();

    void Accepted() { m_isAccepted = true; }

    bool IsAccepted() { return m_isAccepted; }


private:
    bool m_isAccepted;
};

class MORTY_API MMouseInputEvent : public MInputEvent
{
public:
    enum MEMouseDownButton
    {
        NoneButton   = 0,
        LeftButton   = 1 << 0,
        RightButton  = 1 << 1,
        ScrollButton = 1 << 2,
    };

    enum MEMouseInputType
    {
        None       = 0,
        ButtonDown = 1,
        ButtonUp   = 2,
        MouseMove  = 3,
    };

    MMouseInputEvent(const MEMouseDownButton& eMouseDownButton, const MEMouseInputType& eInputType);

    MMouseInputEvent(const Vector2& v2MousePosition, const Vector2& v2MousePositionAddition);

    Vector2           GetMosuePosition() { return s_v2MousePosition; }

    Vector2           GetMouseAddition() { return m_mousePositionAddition; }

    MEMouseDownButton GetButton() { return m_eventButton; }

    MEMouseInputType  GetType() { return m_inputType; }

    static bool       IsButtonDown(const MEMouseDownButton& eButton);

protected:
    MEMouseDownButton m_eventButton           = MEMouseDownButton::NoneButton;
    Vector2           m_mousePositionAddition = {};

    static uint32_t   s_unMouseDownButton;
    static Vector2    s_v2MousePosition;

    MEMouseInputType  m_inputType = MEMouseInputType::None;
};

class MORTY_API MKeyBoardInputEvent : public MInputEvent
{
public:
    MKeyBoardInputEvent(const uint32_t& unKeyIndex, const MEKeyState& eInputType);

    uint32_t    GetKey() { return m_unKeyIndex; }

    MEKeyState  GetType() { return m_inputType; }

    static bool IsKeyDown(const int& nKey);

protected:
    static bool s_vKeyDownMap[256];

    uint32_t    m_unKeyIndex;
    MEKeyState  m_inputType;
};

}// namespace morty