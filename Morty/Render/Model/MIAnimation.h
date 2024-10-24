/**
 * @File         MIAnimation
 * 
 * @Created      2019-12-09 22:37:59
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

class MEngine;
class MORTY_API MIAnimation
{
public:
    MIAnimation()          = default;
    virtual ~MIAnimation() = default;
};

class MORTY_API MIAnimController
{
public:
    enum MEAnimControllerState
    {
        EPlay  = 1,
        EPause = 2,
        EStop  = 3,
    };

public:
    virtual void                  Play()                     = 0;
    virtual void                  Pause()                    = 0;
    virtual void                  Stop()                     = 0;
    virtual void                  SetLoop(const bool& bLoop) = 0;
    virtual bool                  GetLoop()                  = 0;
    virtual MEAnimControllerState GetState()                 = 0;

    virtual void                  NextStep(const float& fDelta, const bool& bAnimStep) = 0;
};

}// namespace morty