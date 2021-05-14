/**
 * @File         MPhysicsWorld
 * 
 * @Created      2021-05-14 15:23:28
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPHYSICSWORLD_H_
#define _M_MPHYSICSWORLD_H_
#include "MGlobal.h"

class btTransform;
class btRigidBody;
class btCollisionShape;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;

class MORTY_API MPhysicsWorld
{
public:
    MPhysicsWorld();
    virtual ~MPhysicsWorld();

public:


	void CreateEmptyDynamicsWorld();

	btRigidBody* CreateRigidBody(float mass, const btTransform* startTransform, btCollisionShape* shape);
	void DeleteRigidBody(btRigidBody* body);

private:

	btBroadphaseInterface* m_broadphase;
	btCollisionDispatcher* m_dispatcher;
	btConstraintSolver* m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btDiscreteDynamicsWorld* m_dynamicsWorld;
};


#endif
