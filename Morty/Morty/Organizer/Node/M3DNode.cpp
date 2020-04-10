#include "M3DNode.h"

MTypeIdentifierImplement(M3DNode, MNode)

M3DNode::M3DNode()
	: MNode()
	, m_transform()
	, m_m4Transform(Matrix4::IdentityMatrix)
	, m_m4WorldTransform(Matrix4::IdentityMatrix)
	, m_m4WorldToLocalTransform(Matrix4::IdentityMatrix)
	, m_bLocalTransformDirty(true)
	, m_bWorldTransformDirty(true)
	, m_bWorldToLocalTransform(true)
{

}

M3DNode::~M3DNode()
{

}

Matrix4 M3DNode::GetParentWorldTransform()
{
	if (m_bWorldTransformDirty)
		UpdateWorldTransform();

	return m_m4WorldTransform;
}

Matrix4 M3DNode::GetWorldToLocalTransform()
{
	if (m_bWorldToLocalTransform)
	{
		m_bWorldToLocalTransform = false;

		m_m4WorldToLocalTransform = GetParentWorldTransform().Inverse();
	}

	return m_m4WorldToLocalTransform;
}

Matrix4 M3DNode::GetWorldTransform()
{
	return GetParentWorldTransform() * GetLocalTransform();
}

Matrix4 M3DNode::GetLocalTransform()
{
	if (m_bLocalTransformDirty)
	{
		m_m4Transform = m_transform.GetMatrix();
		m_bLocalTransformDirty = false;
	}

	return m_m4Transform;
}

void M3DNode::UpdateWorldTransform()
{

	m_bWorldTransformDirty = false;

	m_m4WorldTransform = Matrix4::IdentityMatrix;
	

	//找到最近的3D祖宗节点，拿他的矩阵乘一下
	if (GetParent())
	{
		MNode* pNode = this;
		while (pNode = pNode->GetParent())
		{
			if (M3DNode* p3DNode = dynamic_cast<M3DNode*>(pNode))
			{
				m_m4WorldTransform = p3DNode->GetWorldTransform();
				return;
			}
		}
	}
}

Vector3 M3DNode::GetWorldUp()
{
	return GetParentWorldTransform() * m_transform.GetUp();
}

Vector3 M3DNode::GetWorldForward()
{
	return GetParentWorldTransform() * m_transform.GetForward();
}

Vector3 M3DNode::GetWorldRight()
{
	return GetParentWorldTransform() * m_transform.GetRight();
}

bool M3DNode::AddNodeImpl(MNode* pNode, const MENodeChildType& etype)
{
	if (false == MNode::AddNodeImpl(pNode, etype))
		return false;

	WorldTransformDirtyRecursively(pNode);

	return true;
}

void M3DNode::SetPosition(const Vector3& pos)
{
	m_transform.SetPosition(pos);
	LocalTransformDirty();
}

void M3DNode::SetWorldPosition(const Vector3& pos)
{
	Vector3 localPos = GetWorldToLocalTransform() * pos;

	SetPosition(localPos);
}

Vector3 M3DNode::GetWorldPosition()
{
	return GetWorldTransform() * Vector3(0, 0, 0);
}

void M3DNode::SetRotation(const Quaternion& quat)
{
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

void M3DNode::SetScale(const Vector3& scale)
{
	m_transform.SetScale(scale);
	LocalTransformDirty();
}

void M3DNode::SetTransform(const MTransform& trans)
{
	m_transform = trans;
	LocalTransformDirty();
}

void M3DNode::LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir)
{
	v3UpDir.Normalize();
	Vector3 v3WorldPos = GetWorldPosition();

	Vector3 v3Forward = v3TargetWorldPos - v3WorldPos;
	v3Forward.Normalize();

	Vector3 v3RightDir = v3UpDir.CrossProduct(v3Forward);
	v3RightDir.Normalize();

	v3UpDir = v3Forward.CrossProduct(v3RightDir);
	v3UpDir.Normalize();

	Matrix4 matRotate( v3RightDir.x, v3UpDir.x, v3Forward.x, 0,
					v3RightDir.y, v3UpDir.y, v3Forward.y, 0 ,
					v3RightDir.z, v3UpDir.z, v3Forward.z, 0,
					0, 0, 0, 1);

	Quaternion quat = matRotate.GetRotation();
	quat.Normalize();
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

void M3DNode::WorldTransformDirtyRecursively(MNode* pNode)
{
	if (M3DNode* p3DNode = dynamic_cast<M3DNode*>(pNode))
	{
		if (p3DNode->m_bWorldTransformDirty)
			return;

		p3DNode->WorldTransformDirty();
	}

	for (MNode* pChildNode : pNode->GetChildren())
	{
		WorldTransformDirtyRecursively(pChildNode);
	}

	for (MNode* pChildNode : pNode->GetFixedChildren())
	{
		WorldTransformDirtyRecursively(pChildNode);
	}
}

void M3DNode::WorldTransformDirty()
{
	m_bWorldTransformDirty = true;
	m_bWorldToLocalTransform = true;
}

void M3DNode::LocalTransformDirty()
{
	if (m_bLocalTransformDirty)
		return;

	m_bLocalTransformDirty = true;
	for (MNode* pChildNode : m_vFixedChildren)
		WorldTransformDirtyRecursively(pChildNode);
	for (MNode* pChildNode : m_vChildren)
		WorldTransformDirtyRecursively(pChildNode);
}
