#include "MPhysicsManager.h"

#include "MPhysicsWorld.h"

MPhysicsManager::MPhysicsManager()
{

}

MPhysicsManager::~MPhysicsManager()
{

}

void MPhysicsManager::Tick(const float& fDelta)
{
	for (auto& pair : m_tPhysicsWord)
	{
		pair.second->Tick(fDelta);
	}
}

MPhysicsWorld* MPhysicsManager::GetPhysicsWorld(const uint32_t& unSceneID)
{
	MPhysicsWorld* pWorld = m_tPhysicsWord[unSceneID];

	if (nullptr == pWorld)
	{
		pWorld = new MPhysicsWorld();
		pWorld->CreateEmptyDynamicsWorld();
		m_tPhysicsWord[unSceneID] = pWorld;
	}

	return pWorld;
}
