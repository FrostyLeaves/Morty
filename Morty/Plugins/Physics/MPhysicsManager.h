/**
 * @File         MPhysicsManager
 * 
 * @Created      2021-05-14 15:25:09
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPHYSICSMANAGER_H_
#define _M_MPHYSICSMANAGER_H_
#include "MGlobal.h"

#include <map>

class MPhysicsWorld;
class MORTY_API MPhysicsManager
{
public:
    MPhysicsManager();
    virtual ~MPhysicsManager();

public:

    void Tick(const float& fDelta);

public:

    MPhysicsWorld* GetPhysicsWorld(const uint32_t& unSceneID);
    

private:

    std::map<uint32_t, MPhysicsWorld*> m_tPhysicsWord;

};


#endif
