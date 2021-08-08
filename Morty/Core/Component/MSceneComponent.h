/**
 * @File         MSceneComponent
 * 
 * @Created      2021-04-26 17:17:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSCENECOMPONENT_H_
#define _M_MSCENECOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "MTransform.h"


class MORTY_API MSceneComponent : public MComponent
{
public:
	MORTY_CLASS(MSceneComponent)
public:
    MSceneComponent();
    virtual ~MSceneComponent();


	virtual void Initialize() override;
	virtual void Release() override;

public:
//property

	void SetTransform(const MTransform& trans);
	MTransform GetTransform() { return m_transform; }

	void SetParentComponent(const MComponentID& parent);
	MComponentID GetParentComponent() { return m_attachParent; }

	void SetVisible(const bool& bVisible);
	bool GetVisible() const { return m_bVisible; }

	bool GetVisibleRecursively();

public:
	void SetPosition(const Vector3& pos);
	Vector3 GetPosition() { return m_transform.GetPosition(); }
	
	void SetWorldPosition(const Vector3& pos);
	Vector3 GetWorldPosition();

	void SetWorldRotation(const Quaternion& quat);
	Quaternion GetWorldRotation();

	void SetRotation(const Quaternion& quat);
	Quaternion GetRotation() { return m_transform.GetRotation(); }

	void SetScale(const Vector3& scale);
	Vector3 GetScale() { return m_transform.GetScale(); }

	void SetParent(MSceneComponent* pParent);
	MSceneComponent* GetParent();

	void LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir);

	Matrix4 GetParentWorldTransform();
	Matrix4 GetWorldToLocalTransform();
	Matrix4 GetWorldTransform();
	Matrix4 GetLocalTransform();

	Vector3 GetUp() { return m_transform.GetUp(); }
	Vector3 GetForward() { return m_transform.GetForward(); }
	Vector3 GetRight() { return m_transform.GetRight(); }

	Vector3 GetWorldUp();
	Vector3 GetWorldForward();
	Vector3 GetWorldRight();

public:

	const std::vector<MComponentID>& GetChildrenComponent() const { return m_vAttachChildren; }

	static void CallRecursivelyFunction(MEntity* pEntity, std::function<void(MEntity*)> func);

public:

	virtual void WriteToStruct(MStruct& srt, MComponentRefTable& refTable) override;
	virtual void ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable) override;

protected:

	void LocalTransformDirty();
	void WorldTransformDirty();

	void WorldTransformDirtyRecursively();

	void AddChild(MSceneComponent* pChild);
	void RemoveChild(MSceneComponent* pChild);

	void SetVisibleRecursively(const bool& bVisible);

private:

	MTransform m_transform;
	Matrix4 m_m4Transform;
	Matrix4 m_m4WorldTransform;
	Matrix4 m_m4WorldToLocalTransform;
	bool m_bLocalTransformDirty;
	bool m_bWorldTransformDirty;
	bool m_bWorldToLocalTransformDirty;
	bool m_bVisible;
	bool m_bVisibleRecursively;

	MComponentID m_attachParent;
	std::vector<MComponentID> m_vAttachChildren;
};


#endif
