#include "MRigidBodyComponent.h"

M_OBJECT_IMPLEMENT(MRigidBodyComponent, MComponent)


#include "MNode.h"
#include "MScene.h"
#include "MFunction.h"

#include "MPhysicsWorld.h"
#include "MPhysicsPlugin.h"
#include "MPhysicsManager.h"
#include "MPhysicsFunction.h"

#include "btBulletDynamicsCommon.h"

#include "MSceneComponent.h"

MRigidBodyComponent::MRigidBodyComponent()
	: MComponent()
	, m_pPhysicsWorld(nullptr)
	, m_pRigidBody(nullptr)
	, m_pShape(nullptr)
{

}

MRigidBodyComponent::~MRigidBodyComponent()
{

}

void MRigidBodyComponent::Initialize()
{
	InitializePhysics();

	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
	{
		MLogManager::GetInstance()->Error("Component Initialize, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}

//	pOwnerNode->RegisterComponentNotify<MRigidBodyComponent>(MString("TransformDirty"), M_CLASS_FUNCTION_BIND_0(MRigidBodyComponent::OnTransformDirty));
}

void MRigidBodyComponent::Release()
{
	ReleasePhysics();

	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
	{
		MLogManager::GetInstance()->Error("Component Release, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}

//	pOwnerNode->UnregisterComponentNotify<MRigidBodyComponent>(MString("TransformDirty"));
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

	if (m_pPhysicsWorld && m_pRigidBody)
	{
		m_pPhysicsWorld->AddRigidBody(m_pRigidBody);
	}
}

void MRigidBodyComponent::OnExitScene(MScene* pScene)
{
	Super::OnExitScene(pScene);

	if (m_pPhysicsWorld)
	{
		if (m_pRigidBody)
		{
			m_pPhysicsWorld->RemoveRigidBody(m_pRigidBody);
		}

		m_pPhysicsWorld = nullptr;
	}
}

void MRigidBodyComponent::Tick(const float& fDelta)
{
	Super::Tick(fDelta);

	if (!m_pRigidBody)
		return;

	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
		return;

	MSceneComponent* pSceneComponent = pOwnerNode->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return;

	btTransform transform0 = m_pRigidBody->getWorldTransform();
	MTransform transform1;
	MPhysicsFunction::ConvertTransform(&transform0, transform1);

	pSceneComponent->SetWorldRotation(transform1.GetRotation());
	pSceneComponent->SetWorldPosition(transform1.GetPosition());
}

MPhysicsWorld* MRigidBodyComponent::GetPhysicsWorld()
{
	return m_pPhysicsWorld;
}

void MRigidBodyComponent::InitializePhysics()
{
	if (!m_pShape)
	{
		m_pShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
	}

	if (!m_pRigidBody)
	{
		btTransform transform;

		if (MNode* pOwnerNode = GetOwnerNode())
		{
			if (MSceneComponent* pSceneComponent = pOwnerNode->GetComponent<MSceneComponent>())
			{
				Matrix4 mat4WorldTransform = pSceneComponent->GetWorldTransform();
				MPhysicsFunction::ConvertTransform(mat4WorldTransform, &transform);
			}
		}

		m_pRigidBody = CreateRigidBody(1.0f, &transform, m_pShape);

		if (MPhysicsWorld* pPhysicsWorld = GetPhysicsWorld())
		{
			pPhysicsWorld->AddRigidBody(m_pRigidBody);
		}
	}
}

void MRigidBodyComponent::ReleasePhysics()
{
	if (m_pRigidBody)
	{
		if (MPhysicsWorld* pPhysicsWorld = GetPhysicsWorld())
		{
			pPhysicsWorld->RemoveRigidBody(m_pRigidBody);
		}

		DeleteRigidBody(m_pRigidBody);
		m_pRigidBody = nullptr;
	}

	if (m_pShape)
	{
		delete m_pShape;
		m_pShape = nullptr;
	}
}

btRigidBody* MRigidBodyComponent::CreateRigidBody(float mass, const btTransform* startTransform, btCollisionShape* shape)
{
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	btDefaultMotionState* myMotionState = new btDefaultMotionState(*startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

	body->setUserIndex(-1);
	return body;
}

void MRigidBodyComponent::DeleteRigidBody(btRigidBody* pRigidBody)
{
	if (pRigidBody)
	{
		btMotionState* ms = pRigidBody->getMotionState();
		delete pRigidBody;
		delete ms;
	}
}

void MRigidBodyComponent::WriteToStruct(MStruct& srt)
{

}

void MRigidBodyComponent::ReadFromStruct(const MStruct& srt)
{

}
