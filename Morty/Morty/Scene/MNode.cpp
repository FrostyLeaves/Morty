#include "MNode.h"

MNode::MNode()
	: MObject()
	, m_pParent(nullptr)
{

}

MNode::~MNode()
{

}

bool MNode::AddNode(MNode* pNode)
{
	if (isHolderOf(pNode))
		return false;

	m_vChildren.push_back(pNode);
	pNode->m_pParent = this;

	return true;
}

bool MNode::RemoveNode(MNode* pNode)
{
	for (std::vector<MNode*>::iterator iter = m_vChildren.begin(); iter != m_vChildren.end(); ++iter)
	{
		if (*iter == pNode)
		{
			pNode->m_pParent = nullptr;
			m_vChildren.erase(iter);
			return true;
		}
	}

	return false;
}

bool MNode::isHolderOf(MNode* pNode)
{
	MNode* pParent = pNode;

	while (pParent)
	{
		if (pParent == this)
			return true;

		pParent = pParent->GetParent();
	}

	return false;
}
