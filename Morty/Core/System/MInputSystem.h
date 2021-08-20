/**
 * @File         MInputSystem
 * 
 * @Created      2021-08-20 10:56:12
 *
 * @Author       Pobrecito
**/

#ifndef _M_MINPUTSYSTEM_H_
#define _M_MINPUTSYSTEM_H_
#include "MGlobal.h"
#include "MSystem.h"

#include "Vector.h"
#include "MInputEvent.h"

class MORTY_API MInputSystem : public MISystem
{
    MORTY_CLASS(MInputSystem)
public:
    MInputSystem();
    virtual ~MInputSystem();

public:

    void Input(MInputEvent* pEvent);

    bool IsKeyDown(const int& nKey);

    bool IsMouseButtonDown(const MMouseInputEvent::MEMouseDownButton& eButton);

    Vector2 GetMouseAddition() { return m_v2MouseAddition; }

public:

    virtual void EngineTick(const float& fDelta) override;

private:

    Vector2 m_v2MouseAddition;
};


#endif
