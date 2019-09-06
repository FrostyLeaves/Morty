#include "M3DNode.h"

M3DNode::M3DNode()
	: MNode()
	, m_m4Transform(IdentityMatrix)
	, m_bWorldTransformDirty(true)
	, m_m4WorldTransform(IdentityMatrix)
{

}

M3DNode::~M3DNode()
{

}

Matrix4 M3DNode::GetWorldTransform()
{
	if (m_bWorldTransformDirty)
		UpdateWorldTransform();
	
	return m_m4WorldTransform * m_m4Transform;
}

void M3DNode::UpdateWorldTransform()
{

	m_bWorldTransformDirty = false;

	m_m4WorldTransform = IdentityMatrix;
	

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

void M3DNode::SetPosition(const Vector3& pos)
{
	m_m4Transform.SetTranslation(pos.x, pos.y, pos.z);
	WorldTransformDirty();
}

Vector3 M3DNode::GetPosition()
{
	return m_m4Transform.GetTranslation();
}

void M3DNode::SetRotation(const Quaternion& quat)
{
	m_m4Transform.SetRotation(quat);
	WorldTransformDirty();
}

Quaternion M3DNode::GetRotation()
{
	return m_m4Transform.GetRotation();
}

void M3DNode::WorldTransformDirty()
{
	m_bWorldTransformDirty = true;
	for (MNode* pNode : m_vChildren)
	{
		if (M3DNode* p3DChild = dynamic_cast<M3DNode*>(pNode))
			p3DChild->WorldTransformDirty();
	}
}
