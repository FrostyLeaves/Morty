#include "MNode.h"
#include "MScene.h"

#include <queue>

MTypeIdentifierImplement(MNode, MObject)

MNode::MNode()
	: MObject()
	, m_pParent(nullptr)
	, m_pScene(nullptr)
	, m_bVisibleRecursively(true)
	, m_bVisible(true)
{

}

MNode::~MNode()
{

}

void MNode::UpdateVisibleRecursively()
{
	bool bParentVisible = m_pParent ? m_pParent->m_bVisibleRecursively : true;

	m_bVisibleRecursively = bParentVisible & m_bVisible;
	for (MNode* pChild : m_vFixedChildren)
		pChild->UpdateVisibleRecursively();
	for (MNode* pChild : m_vChildren)
		pChild->UpdateVisibleRecursively();
}

void MNode::SetVisible(const bool& bVisible)
{
	if (m_bVisible == bVisible)
		return;

	m_bVisible = bVisible;

	bool bParentVisible = m_pParent ? m_pParent->m_bVisibleRecursively : true;

	if ((bParentVisible & bVisible) != m_bVisibleRecursively)
		UpdateVisibleRecursively();

}

void MNode::SetAttachedScene(MScene* pScene)
{
	//different scene.
	if (this->m_pScene != pScene)
	{
		std::queue<MNode*> que;
		que.push(this);

		while (!que.empty())
		{
			MNode* pFrontNode = que.front();
			que.pop();

			for (MNode* pChildNode : pFrontNode->GetChildren())
				que.push(pChildNode);
			for (MNode* pChildNode : pFrontNode->GetFixedChildren())
				que.push(pChildNode);

			if (pFrontNode->m_pScene)
				pFrontNode->m_pScene->OnNodeExit(pFrontNode);

			if (pFrontNode->m_pScene = pScene)
				pScene->OnNodeEnter(pFrontNode);
		}
	}
}

bool MNode::AddNodeImpl(MNode* pNode, const MENodeChildType& etype)
{
	if (pNode->isHolderOf(this))
		return false;

	std::vector<MNode*>& children = etype == MENodeChildType::EFixed ? m_vFixedChildren : m_vChildren;

	for (MNode* pChildNode : children)
	{
		if (pChildNode == pNode)
			return false;
	}

	children.push_back(pNode);
	pNode->m_pParent = this;
	
	//Update Attached Scene.
	pNode->SetAttachedScene(m_pScene);
	
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

std::vector<MNode*> MNode::FindChildrenByName(const MString& strName)
{
	std::vector<MNode*> vResult;
	FindChildrenByName(strName, vResult);
	return vResult;
}

void MNode::FindChildrenByName(const MString& strName, std::vector<MNode*>& vNodes)
{
	for (MNode* pNode : m_vChildren)
	{
		if (pNode->m_strName == strName)
			vNodes.push_back(pNode);
		pNode->FindChildrenByName(strName, vNodes);
	}
}

std::vector<MNode*> MNode::FindChildrenByFunc(const SearchNodeFunction& func)
{
	std::vector<MNode*> vResult;
	if (func)
		FindChildrenByFunc(func, vResult);
	return vResult;
}

void MNode::FindChildrenByFunc(const SearchNodeFunction& func, std::vector<MNode*>& vNodes)
{
	for (MNode* pNode : m_vChildren)
	{
		if(func(pNode))
			vNodes.push_back(pNode);
		pNode->FindChildrenByFunc(func, vNodes);
	}
}

bool MNode::RemoveNodeImpl(MNode* pNode, const MENodeChildType& etype)
{
	std::vector<MNode*>& children = etype == MENodeChildType::EFixed ? m_vFixedChildren : m_vChildren;

	for (std::vector<MNode*>::iterator iter = children.begin(); iter != children.end(); ++iter)
	{
		if (*iter == pNode)
		{
			pNode->m_pParent = nullptr;
			children.erase(iter);

			pNode->SetAttachedScene(nullptr);

			return true;
		}
	}

	return false;
}

void MNode::RemoveAllNodeImpl(const MENodeChildType& etype, const float& bDelete/* = false*/)
{
	std::vector<MNode*>& children = etype == MENodeChildType::EFixed ? m_vFixedChildren : m_vChildren;

	if (bDelete)
	{
		for (MNode* pChild : children)
			pChild->DeleteLater();
	}
	else
	{
		for (MNode* pChild : children)
		{
			pChild->m_pParent = nullptr;
			pChild->SetAttachedScene(nullptr);
		}
	}

	children.clear();
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
	if (m_bDeleteMark) return;

	OnTick(fDelta);

	for (MNode* pChild : m_vFixedChildren)
		pChild->Tick(fDelta);

	for (MNode* pChild : m_vChildren)
		pChild->Tick(fDelta);
}

void MNode::OnDelete()
{
	// remove from parent [and scene]
	if (m_pParent)
		m_pParent->RemoveNode(this);

	RemoveAllNodeImpl(MENodeChildType::ENormal, true);
	RemoveAllNodeImpl(MENodeChildType::EFixed, true);

	Super::OnDelete();
}

void MNode::OnTick(const float& fDelta)
{

}
