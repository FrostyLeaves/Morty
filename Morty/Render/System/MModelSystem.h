/**
 * @File         MModelSystem
 * 
 * @Created      2021-07-21 14:37:08
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMODELSYSTEM_H_
#define _M_MMODELSYSTEM_H_
#include "MGlobal.h"
#include "MSystem.h"

class MScene;
class MORTY_API MModelSystem : public MISystem
{
    MORTY_CLASS(MModelSystem)
public:
    MModelSystem();
    virtual ~MModelSystem();

public:

    void UpdateAnimation(MScene* pScene, const float& fDelta);

private:

};


#endif
