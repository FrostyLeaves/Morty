#include "MNode.h"
#include "MScene.h"
#include "MVariant.h"
#include "Json/MJson.h"

#include <queue>

M_OBJECT_IMPLEMENT(MNode, MObject) 

MNode::MNode()
	: MObject()
	, m_eChildType(MENodeChildType::ENormal)
	, m_pParent(nullptr)
	, m_pScene(nullptr)
	, m_bVisibleRecursively(true)
	, m_strName("Node")
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
	pNode->m_eChildType = etype;
	
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

void MNode::RemoveAllNodeImpl(const MENodeChildType& etype)
{
	std::vector<MNode*>& children = etype == MENodeChildType::EFixed ? m_vFixedChildren : m_vChildren;

	for (MNode* pChild : children)
	{
		pChild->m_pParent = nullptr;
		pChild->SetAttachedScene(nullptr);
		pChild->DeleteLater();
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
	if (m_pParent && !m_pParent->m_bDeleteMark)
		m_pParent->RemoveNodeImpl(this, m_eChildType);
	else if (m_pScene) // as root node
		this->SetAttachedScene(nullptr);

	RemoveAllNodeImpl(MENodeChildType::ENormal);
	RemoveAllNodeImpl(MENodeChildType::EFixed);

	Super::OnDelete();
}

void MNode::WriteToStruct(MStruct& srt)
{
	if (MString* pString = FindWriteVariant<MString>(srt, "NodeType"))
	{
		*pString = GetTypeName();
	}

	if (MStruct* pStruct = FindWriteVariant<MStruct>(srt, MNode::GetClassTypeName()))
	{
		pStruct->AppendMVariant("m_strName", m_strName);
		pStruct->AppendMVariant("m_bVisible", m_bVisible);
	}

	WriteChildrenToStruct(srt);
}

void MNode::ReadFormStruct(MStruct& srt)
{
	if (MStruct* pStruct = FindReadVariant<MStruct>(srt, MNode::GetClassTypeName()))
	{
		if (MString* pName = pStruct->FindMember("m_strName")->GetString())
			SetName(*pName);

		if (bool bVislble = pStruct->FindMember("m_bVisible")->IsTrue())
			SetVisible(bVislble);
	}


	//ReadChildrenToStruct(srt);
}

void MNode::WriteChildrenToStruct(MStruct& srt)
{
	if (MVariantArray* pArray = FindWriteVariant<MVariantArray>(srt, "Children"))
	{
		unsigned int unSize = m_vChildren.size();
		pArray->Resize(unSize);
		for (unsigned int i = 0; i < unSize; ++i)
		{
			(*pArray)[i] = MStruct();
			MStruct& childSrt = *((*pArray)[i].GetVar<MStruct>());
			m_vChildren[i]->WriteToStruct(childSrt);
		}
	}

	if (MVariantArray* pArray = FindWriteVariant<MVariantArray>(srt, "FixedChildren"))
	{
		unsigned int unSize = m_vFixedChildren.size();
		pArray->Resize(unSize);
		for (unsigned int i = 0; i < unSize; ++i)
		{
			(*pArray)[i] = MStruct();
			m_vFixedChildren[i]->WriteToStruct(*((*pArray)[i].GetVar<MStruct>()));
		}
	}
}

void MNode::ReadChildrenFromStruct(MStruct& srt)
{
	if (MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "Children"))
	{
		unsigned int unSize = pArray->GetSize();
		m_vChildren.resize(unSize);
		for (unsigned int i = 0; i < unSize; ++i)
		{
			MStruct& childSrt = *(*pArray)[i].GetStruct();
			if (MString* pString = FindWriteVariant<MString>(childSrt, "NodeType"))
			{
				if (MObject* pChildObject = m_pEngine->GetObjectManager()->CreateObject(*pString))
				{
					if (MNode* pChildNode = pChildObject->DynamicCast<MNode>())
					{
						pChildNode->ReadFormStruct(childSrt);
						m_vChildren[i] = pChildNode;
					}
				}
			}
		}
	}
	if (MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "FixedChildren"))
	{
		unsigned int unSize = pArray->GetSize();
		m_vFixedChildren.resize(unSize);
		for (unsigned int i = 0; i < unSize; ++i)
		{
			MStruct& childSrt = *(*pArray)[i].GetStruct();
			if (MString* pString = FindWriteVariant<MString>(childSrt, "NodeType"))
			{
				if (MObject* pChildObject = m_pEngine->GetObjectManager()->CreateObject(*pString))
				{
					if (MNode* pChildNode = pChildObject->DynamicCast<MNode>())
					{
						pChildNode->ReadFormStruct(childSrt);
						m_vFixedChildren[i] = pChildNode;
					}
				}
			}
		}
	}
}

void MNode::Encode(MString& strCode)
{
	MVariant var = MStruct();
	MStruct* pStruct = var.GetStruct();

	WriteToStruct(*pStruct);

	MJson::MVariantToJson(var, strCode);
}

void MNode::Decode(MString& strCode)
{
	MVariant var;
	MJson::JsonToMVariant(strCode, var);

	ReadFormStruct(*var.GetStruct());
}

void MNode::OnTick(const float& fDelta)
{

}
