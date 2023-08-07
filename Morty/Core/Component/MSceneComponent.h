/**
 * @File         MSceneComponent
 * 
 * @Created      2021-04-26 17:17:56
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Utility/MTransform.h"


class MORTY_API MSceneComponent : public MComponent
{
public:
	MORTY_CLASS(MSceneComponent)
public:
    MSceneComponent();
    ~MSceneComponent() override;


public:
//property

	void SetTransform(const MTransform& trans);
	MTransform GetTransform() { return m_transform; }

	void SetParentComponent(const MComponentID& parent);
	MComponentID GetParentComponent() const { return m_attachParent; }

	void SetVisible(const bool& bVisible);
	bool GetVisible() const { return m_bVisible; }

	bool GetVisibleRecursively();

public:
	void SetPosition(const Vector3& pos);
	Vector3 GetPosition() const { return m_transform.GetPosition(); }
	
	void SetWorldPosition(const Vector3& pos);
	Vector3 GetWorldPosition();

	void SetWorldRotation(const Quaternion& quat);
	Quaternion GetWorldRotation();

	void SetWorldScale(const Vector3 scale);
	Vector3 GetWorldScale();

	void SetRotation(const Quaternion& quat);
	Quaternion GetRotation() const { return m_transform.GetRotation(); }

	void SetScale(const Vector3& scale);
	Vector3 GetScale() const { return m_transform.GetScale(); }

	void SetParent(MSceneComponent* pParent);
	MSceneComponent* GetParent();

	void LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir);

	Matrix4 GetParentWorldTransform();
	Matrix4 GetWorldToLocalTransform();
	Matrix4 GetWorldTransform();
	Matrix4 GetLocalTransform();

	Vector3 GetUp() const { return m_transform.GetUp(); }
	Vector3 GetForward() const { return m_transform.GetForward(); }
	Vector3 GetRight() const { return m_transform.GetRight(); }

	Vector3 GetWorldUp();
	Vector3 GetWorldForward();
	Vector3 GetWorldRight();

public:

	const std::vector<MComponentID>& GetChildrenComponent() const { return m_vAttachChildren; }

	static void CallRecursivelyFunction(MEntity* pEntity, std::function<void(MEntity*)> func);

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;
	virtual void PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid) override;

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

	MGuid m_parentGuid;
	MComponentID m_attachParent;
	std::vector<MComponentID> m_vAttachChildren;
};
