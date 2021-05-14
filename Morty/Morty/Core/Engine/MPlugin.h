/**
 * @File         MPlugin
 * 
 * @Created      2021-05-14 14:24:36
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPLUGIN_H_
#define _M_MPLUGIN_H_
#include "MGlobal.h"
#include "MTypedClass.h"

class MObject;
class MORTY_API MIPlugin : public MTypedClass
{
public:
    M_I_OBJECT(MIPlugin)
public:
    MIPlugin();
    virtual ~MIPlugin();

public:

    virtual bool Initialize() = 0;
    virtual void Release() = 0;

    virtual void Tick(const float& fDelta) = 0;


public:

    virtual void OnObjectCreate(MObject* pObject) {}
    virtual void OnObjectDestroy(MObject* pObject) {}

private:

};


#endif
