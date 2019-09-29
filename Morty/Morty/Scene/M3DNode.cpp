#include "M3DNode.h"

M3DNode::M3DNode()
	: MNode()
	, m_transform()
	, m_bLocalTransformDirty(true)
	, m_m4Transform(Matrix4::IdentityMatrix)
	, m_bWorldTransformDirty(true)
	, m_m4WorldTransform(Matrix4::IdentityMatrix)
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

Matrix4 M3DNode::GetWorldTransform()
{
	return GetLocalTransform() * GetParentWorldTransform();
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
				m_m4WorldTransform = m_m4WorldTransform * p3DNode->GetWorldTransform();
				return;
			}
		}
	}
}

bool M3DNode::AddNode(MNode* pNode)
{
	if (false == MNode::AddNode(pNode))
		return false;

	WorldTransformDirty(pNode);

	return true;
}

void M3DNode::SetPosition(const Vector3& pos)
{
	m_transform.SetPosition(pos);
	LocalTransformDirty();
}

void M3DNode::SetWorldPosition(const Vector3& pos)
{
	Vector3 v3LocalPos = pos - GetWorldPosition();
	SetPosition(v3LocalPos);
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

void M3DNode::LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir)
{
	v3UpDir.Normalization();
	Vector3 v3WorldPos = GetWorldPosition();

	Vector3 v3Forward = v3TargetWorldPos - v3WorldPos;
	v3Forward.Normalization();

	Vector3 v3RightDir = v3UpDir.CrossProduct(v3Forward);
	v3RightDir.Normalization();

	v3UpDir = v3Forward.CrossProduct(v3RightDir);
	v3UpDir.Normalization();

	Matrix4 matRotate( v3RightDir.x, v3UpDir.x, v3Forward.x, 0,
					v3RightDir.y, v3UpDir.y, v3Forward.y, 0 ,
					v3RightDir.z, v3UpDir.z, v3Forward.z, 0,
					0, 0, 0, 1);

	Quaternion quat = matRotate.GetRotation();
	quat.Normalize();
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

void M3DNode::WorldTransformDirty(MNode* pNode)
{
	if (M3DNode* p3DNode = dynamic_cast<M3DNode*>(pNode))
	{
		if (p3DNode->m_bWorldTransformDirty)
			return;
			
		p3DNode->m_bWorldTransformDirty = true;
	}

	for (MNode* pChildNode : pNode->GetChildren())
	{
		WorldTransformDirty(pChildNode);
	}
}

void M3DNode::LocalTransformDirty()
{
	if (m_bLocalTransformDirty)
		return;

	m_bLocalTransformDirty = true;
	for (MNode* pChildNode : m_vChildren)
		WorldTransformDirty(pChildNode);
}
