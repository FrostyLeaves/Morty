#include "MNode.h"

MNode::MNode()
	: MObject()
	, m_pParent(nullptr)
	, m_bVisible(true)
{

}

MNode::~MNode()
{

}

void MNode::SetVisible(const bool& bVisible)
{
	m_bVisible = bVisible;
}

bool MNode::AddNode(MNode* pNode)
{
	if (isHolderOf(pNode))
		return false;

	m_vChildren.push_back(pNode);
	pNode->m_pParent = this;

	return true;
}

MNode* MNode::GetRootNode()
{
	MNode* pNode = this->GetParent();
	if (nullptr == pNode)
		return pNode;

	while (pNode->GetParent())
		pNode = pNode->GetParent();

	return pNode;
}

MNode* MNode::FindFirstChildByName(const MString& strName)
{
	for (MNode* pNode : m_vChildren)
	{
		if (pNode->m_strName == strName)
			return pNode;

		if (MNode* pFindResult = pNode->FindFirstChildByName(strName))
			return pFindResult;
	}

	return nullptr;
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

void MNode::Tick(const float& fDelta)
{
	OnTick(fDelta);

	for (MNode* pChild : m_vChildren)
		pChild->Tick(fDelta);
}

void MNode::Render()
{

}

void MNode::OnTick(const float& fDelta)
{

}
