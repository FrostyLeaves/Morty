#include "MRigidBodyComponent.h"

M_OBJECT_IMPLEMENT(MRigidBodyComponent, MComponent)


#include "MNode.h"
#include "MScene.h"
#include "MPhysicsWorld.h"
#include "MPhysicsPlugin.h"
#include "MPhysicsManager.h"

#include "btBulletDynamicsCommon.h"

MRigidBodyComponent::MRigidBodyComponent()
	: MComponent()
	, m_pPhysicsWorld(nullptr)
{

}

MRigidBodyComponent::~MRigidBodyComponent()
{

}

void MRigidBodyComponent::Initialize()
{
	InitializePhysics();
}

void MRigidBodyComponent::Release()
{
	ReleasePhysics();
}

void MRigidBodyComponent::OnEnterScene(MScene* pScene)
{
	Super::OnEnterScene(pScene);

	MEngine* pEngine = GetEngine();
	if (!pEngine)
		return;

	MPhysicsPlugin* pPhysicsPlugin = pEngine->GetPlugin<MPhysicsPlugin>();
	if (!pPhysicsPlugin)
		return;

	MPhysicsManager* pPhysicsManager = pPhysicsPlugin->GetPhysicsManager();
	if (!pPhysicsManager)
		return;

	m_pPhysicsWorld = pPhysicsManager->GetPhysicsWorld(pScene->GetObjectID());
}

void MRigidBodyComponent::OnExitScene(MScene* pScene)
{
	Super::OnExitScene(pScene);
}

MPhysicsWorld* MRigidBodyComponent::GetPhysicsWorld()
{
	return m_pPhysicsWorld;
}

void MRigidBodyComponent::InitializePhysics()
{
	btVector3 localInertia(0, 0, 0);

 	btRigidBody::btRigidBodyConstructionInfo cInfo(1.0f, nullptr, nullptr);
 
 	btRigidBody* body = new btRigidBody(cInfo);
}

void MRigidBodyComponent::ReleasePhysics()
{

}

void MRigidBodyComponent::WriteToStruct(MStruct& srt)
{

}

void MRigidBodyComponent::ReadFromStruct(const MStruct& srt)
{

}
