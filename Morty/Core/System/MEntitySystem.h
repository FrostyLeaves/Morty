/**
 * @File         MEntitySystem
 * 
 * @Created      2021-07-20 14:06:53
 *
 * @Author       Pobrecito
**/

#ifndef _M_MENTITYSYSTEM_H_
#define _M_MENTITYSYSTEM_H_
#include "MGlobal.h"
#include "MSystem.h"

class MScene;
class MEntity;
class MResource;
class MORTY_API MEntitySystem : public MISystem
{
    MORTY_CLASS(MEntitySystem)
public:
    MEntitySystem();
    virtual ~MEntitySystem();

public:

    void AddChild(MEntity* pParent, MEntity* pChild);

    MResource* PackEntity(MEntity* pEntity);

    MEntity* LoadEntity(MScene* pScene, MResource* pResource);

private:

};


#endif
