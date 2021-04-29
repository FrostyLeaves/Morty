#include "MRigidBodyComponent.h"

M_OBJECT_IMPLEMENT(MRigidBodyComponent, MComponent)

#include "btBulletDynamicsCommon.h"

MRigidBodyComponent::MRigidBodyComponent()
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
