/**
 * @File         MInputSystem
 * 
 * @Created      2021-08-20 10:56:12
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

#include "Input/MInputEvent.h"
#include "Math/Vector.h"

namespace morty
{

class MORTY_API MInputSystem : public MISystem
{
    MORTY_CLASS(MInputSystem)
public:
    MInputSystem();

    virtual ~MInputSystem();

public:
    void    Input(MInputEvent* pEvent);

    bool    IsKeyDown(const int& nKey);

    bool    IsMouseButtonDown(const MMouseInputEvent::MEMouseDownButton& eButton);

    Vector2 GetMouseAddition() { return m_mouseAddition; }

public:
    virtual void EngineTick(const float& fDelta) override;

private:
    Vector2 m_mouseAddition;
};

}// namespace morty