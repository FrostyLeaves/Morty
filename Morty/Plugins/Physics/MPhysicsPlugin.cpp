#include "MPhysicsPlugin.h"

M_OBJECT_IMPLEMENT(MPhysicsPlugin, MIPlugin)

#include "MPhysicsManager.h"

MPhysicsPlugin::MPhysicsPlugin()
	: MIPlugin()
	, m_pPhysicsManager(nullptr)
{

}

MPhysicsPlugin::~MPhysicsPlugin()
{

}

bool MPhysicsPlugin::Initialize()
{
	if (!m_pPhysicsManager)
	{
		m_pPhysicsManager = new MPhysicsManager();
	}


	return true;
}

void MPhysicsPlugin::Release()
{
	if (m_pPhysicsManager)
	{
		delete m_pPhysicsManager;
		m_pPhysicsManager = nullptr;
	}

}

void MPhysicsPlugin::Tick(const float& fDelta)
{

}

MPhysicsManager* MPhysicsPlugin::GetPhysicsManager()
{
	return m_pPhysicsManager;
}
