#include "MNode.h"
#include "MIScene.h"

#include <queue>

MNode::MNode()
	: MObject()
	, m_pParent(nullptr)
	, m_pScene(nullptr)
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

void MNode::SetAttachedScene(MIScene* pScene)
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

			if (pFrontNode->m_pScene)
				pFrontNode->m_pScene->OnRemoveNode(pFrontNode);

			if (pFrontNode->m_pScene = pScene)
				pScene->OnAddNode(pFrontNode);
		}
	}
}

bool MNode::AddNode(MNode* pNode)
{
	if (pNode->isHolderOf(this))
		return false;

	for (MNode* pChildNode : GetChildren())
	{
		if (pChildNode == pNode)
			return false;
	}

	m_vChildren.push_back(pNode);
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

bool MNode::RemoveNode(MNode* pNode)
{
	for (std::vector<MNode*>::iterator iter = m_vChildren.begin(); iter != m_vChildren.end(); ++iter)
	{
		if (*iter == pNode)
		{
			pNode->m_pParent = nullptr;
			m_vChildren.erase(iter);

			pNode->SetAttachedScene(nullptr);

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
