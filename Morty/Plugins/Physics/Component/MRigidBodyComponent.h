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

class MPhysicsWorld;
class MORTY_API MRigidBodyComponent : public MComponent
{
public:
    M_OBJECT(MRigidBodyComponent)

public:
    MRigidBodyComponent();
    virtual ~MRigidBodyComponent();

public:

    virtual void Initialize() override;
    virtual void Release() override;

	virtual void OnEnterScene(MScene* pScene) override;
	virtual void OnExitScene(MScene* pScene) override;

public:

	MPhysicsWorld* GetPhysicsWorld();

	void InitializePhysics();
	void ReleasePhysics();

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(const MStruct& srt) override;

private:


	MPhysicsWorld* m_pPhysicsWorld;
};


#endif
