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

Matrix4 M3DNode::GetWorldTransform()
{
	if (m_bWorldTransformDirty)
		UpdateWorldTransform();
	
	return m_m4WorldTransform * GetLocalTransform();
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
				m_m4WorldTransform = p3DNode->GetWorldTransform() * m_m4WorldTransform;
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

Vector3 M3DNode::GetPosition()
{
	return m_transform.GetPosition();
}

void M3DNode::SetRotation(const Quaternion& quat)
{
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

Quaternion M3DNode::GetRotation()
{
	return m_transform.GetRotation();
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
