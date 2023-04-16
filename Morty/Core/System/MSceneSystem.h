/**
 * @File         MSceneSystem
 * 
 * @Created      2021-07-21 14:19:06
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSCENESYSTEM_H_
#define _M_MSCENESYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

class MEntity;
class MORTY_API MSceneSystem : public MISystem
{
    MORTY_CLASS(MSceneSystem)
public:
    MSceneSystem();
    virtual ~MSceneSystem();

public:

    void SetVisible(MEntity* pEntity, const bool& bVisible);

private:

};


#endif
