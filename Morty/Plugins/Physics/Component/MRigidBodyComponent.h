/**
 * @File         MRigidBodyComponent
 * 
 * @Created      2021-04-26 16:33:27
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRIGIDBODYCOMPONENT_H_
#define _M_MRIGIDBODYCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

class btTransform;
class btRigidBody;
class btCollisionShape;
class MPhysicsWorld;
class MORTY_API MRigidBodyComponent : public MComponent
{
public:
    M_OBJECT(MRigidBodyComponent)

public:
    MRigidBodyComponent();
    virtual ~MRigidBodyComponent();

public:

	void SetWorldPosition(const Vector3& position);
	Vector3 GetWorldPosition() const;

	void SetWorldRotation(const Quaternion& quat);
	Quaternion GetWorldRotation() const;


public:

    virtual void Initialize() override;
    virtual void Release() override;

	virtual void OnEnterScene(MScene* pScene) override;
	virtual void OnExitScene(MScene* pScene) override;


	virtual void Tick(const float& fDelta) override;

public:

	MPhysicsWorld* GetPhysicsWorld();

	void InitializePhysics();
	void ReleasePhysics();

	btRigidBody* CreateRigidBody(float mass, const btTransform* startTransform, btCollisionShape* shape);
	void DeleteRigidBody(btRigidBody* pRigidBody);

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(const MStruct& srt) override;

private:


	MPhysicsWorld* m_pPhysicsWorld;
	btRigidBody* m_pRigidBody;
	btCollisionShape* m_pShape;
};


#endif
