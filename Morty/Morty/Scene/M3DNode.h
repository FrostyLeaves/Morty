/**
 * @File         M3DNode
 * 
 * @Created      2019-05-25 19:54:30
 *
 * @Author       Morty
**/

#ifndef _M_M3DNODE_H_
#define _M_M3DNODE_H_
#include "MGlobal.h"

#include "MNode.h"

#include "Quaternion.h"
#include "MTransform.h"

class MORTY_CLASS M3DNode : public MNode
{
public:
	M_OBJECT(M3DNode);
    M3DNode();
    virtual ~M3DNode();


	void SetPosition(const Vector3& pos);
	Vector3 GetPosition() { return m_transform.GetPosition(); }
	void SetWorldPosition(const Vector3& pos);
	Vector3 GetWorldPosition();

	void SetRotation(const Quaternion& quat);
	Quaternion GetRotation(){ return m_transform.GetRotation(); }

	void SetScale(const Vector3& scale);
	Vector3 GetScale() { return m_transform.GetScale(); }

	void LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir);

	Matrix4 GetParentWorldTransform();
	Matrix4 GetWorldTransform();
	Matrix4 GetLocalTransform();

	Vector3 GetUp() { return m_transform.GetUp(); }
	Vector3 GetForward() { return m_transform.GetForward(); }
	Vector3 GetRight() { return m_transform.GetRight(); }

public:

	virtual bool AddNode(MNode* pNode) override;

public:
	void UpdateWorldTransform();
protected:
	static void WorldTransformDirty(MNode* pNode);
	void LocalTransformDirty();

private:

	MTransform m_transform;
	bool m_bLocalTransformDirty;
	Matrix4 m_m4Transform;
	bool m_bWorldTransformDirty;
	Matrix4 m_m4WorldTransform;
};


#endif
