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
	
	return m_m4WorldTransform;
}

void M3DNode::UpdateWorldTransform()
{

	m_bWorldTransformDirty = false;

	m_m4WorldTransform = m_m4Transform;
	

	//找到最近的3D祖宗节点，拿他的矩阵乘一下
	if (GetParent())
	{
		MNode* pNode = this;
		while (pNode = pNode->GetParent())
		{
			if (M3DNode* p3DNode = dynamic_cast<M3DNode*>(pNode))
			{
				m_m4WorldTransform = p3DNode->GetWorldTransform() * m_m4Transform;
				return;
			}
		}
	}
}

void M3DNode::SetPosition(const Vector3& pos)
{
	m_m4Transform.SetTranslation(pos.x, pos.y, pos.z);
}

Vector3 M3DNode::GetPosition()
{
	return Vector3();
}
