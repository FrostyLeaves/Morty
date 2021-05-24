#include "MPhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"


MPhysicsWorld::MPhysicsWorld()
	: m_broadphase(nullptr)
	, m_dispatcher(nullptr)
	, m_solver()
	, m_collisionConfiguration()
	, m_dynamicsWorld(nullptr)
{

}

MPhysicsWorld::~MPhysicsWorld()
{

}

void MPhysicsWorld::CreateEmptyDynamicsWorld()
{
	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	m_solver = sol;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

	m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

void MPhysicsWorld::AddRigidBody(btRigidBody* pRigidBody)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->addRigidBody(pRigidBody);
	}
}

void MPhysicsWorld::RemoveRigidBody(btRigidBody* pRigidBody)
{
	m_dynamicsWorld->removeRigidBody(pRigidBody);
}

void MPhysicsWorld::Tick(const float& fDelta)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(fDelta, 10);
	}
}
