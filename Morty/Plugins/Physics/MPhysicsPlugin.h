/**
 * @File         MPhysicsPlugin
 * 
 * @Created      2021-05-14 14:21:58
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPHYSICS_PLUGIN_H_
#define _M_MPHYSICS_PLUGIN_H_
#include "MGlobal.h"
#include "MPlugin.h"

class MPhysicsManager;
class MORTY_API MPhysicsPlugin : public MIPlugin
{
public:
	M_OBJECT(MPhysicsPlugin)
public:
	MPhysicsPlugin();
    virtual ~MPhysicsPlugin();

public:

	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void Tick(const float& fDelta) override;


public:

	MPhysicsManager* GetPhysicsManager();

private:

	MPhysicsManager* m_pPhysicsManager;
};


#endif
