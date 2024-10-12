/**
 * @File         MInputComponent
 * 
 * @Created      2021-04-27 14:19:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

namespace morty
{

class MViewport;
class MInputEvent;
class MORTY_API MInputComponent : public MComponent
{
public:
    MORTY_CLASS(MInputComponent)
public:
    MInputComponent();

    virtual ~MInputComponent();

    typedef std::function<bool(MInputComponent*, MInputEvent*, MViewport*)> MInputCallback;

public:
    void         SetInputCallback(const MInputCallback& func) { m_funcInputCallback = func; }

    virtual bool Input(MInputEvent* pEvent, MViewport* pViewport);

private:
    MInputCallback m_funcInputCallback;
};

}// namespace morty