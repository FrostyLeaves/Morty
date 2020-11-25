#include "MNode.h"
#include "MScene.h"
#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"
#include "Node/MNodeResource.h"

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
	pNode->ParentChangeImpl(this);
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
			pNode->ParentChangeImpl(nullptr);
			children.erase(iter);

			pNode->SetAttachedScene(nullptr);

			return true;
		}
	}

	return false;
}

void MNode::ParentChangeImpl(MNode* pParent)
{
	m_pParent = pParent;
	UpdateVisibleRecursively();
}

void MNode::RemoveAllNodeImpl(const MENodeChildType& etype)
{
	std::vector<MNode*>& children = etype == MENodeChildType::EFixed ? m_vFixedChildren : m_vChildren;

	for (MNode* pChild : children)
	{
		pChild->ParentChangeImpl(nullptr);
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

bool MNode::Load(MResource* pResource)
{
	if (MNodeResource* pNodeResource = pResource->DynamicCast<MNodeResource>())
	{
		MString code;
		if (false == MFileHelper::ReadString(pNodeResource->GetResourcePath(), code))
		{
			MLogManager::GetInstance()->Warning("Node Load File Failed: %s.", pNodeResource->GetResourcePath().c_str());
			return false;
		}

		Decode(code);
		return true;
	}

	return false;
}

MNode* MNode::CreateNodeByVariant(MEngine* pEngine, MStruct& srt)
{
	if (MString* pString = FindReadVariant<MString>(srt, "NodeType"))
	{
		if (MObject* pObject = pEngine->GetObjectManager()->CreateObject(*pString))
		{
			if (MNode* pNode = pObject->DynamicCast<MNode>())
			{
				pNode->ReadFromStruct(srt);
				return pNode;
			}
		}
	}

	return nullptr;
}

void MNode::WriteToStruct(MStruct& srt)
{
	MSerializer::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);

	M_SERIALIZER_WRITE_VALUE("Name", GetName);
	M_SERIALIZER_WRITE_VALUE("Visible", GetVisible);

	M_SERIALIZER_END;
		
	if (MString* pString = FindWriteVariant<MString>(srt, "NodeType"))
	{
		*pString = GetTypeName();
	}

	WriteChildrenToStruct(srt);
}

void MNode::ReadFromStruct(MStruct& srt)
{
	MSerializer::ReadFromStruct(srt);

	M_SERIALIZER_BEGIN(Read);

	M_SERIALIZER_READ_VALUE("Name", SetName, String);
	M_SERIALIZER_READ_VALUE("Visible", SetVisible, Bool);

	M_SERIALIZER_END;

	ReadChildrenFromStruct(srt);
}

void MNode::WriteChildrenToStruct(MStruct& srt)
{
	if (MVariantArray* pArray = FindWriteVariant<MVariantArray>(srt, "Children"))
	{
		uint32_t unSize = m_vChildren.size();

		for (uint32_t i = 0; i < unSize; ++i)
		{
			pArray->AppendMVariant(MStruct());
			MStruct& childSrt = (*pArray)[i].GetVarUnsafe<MStruct>();
			m_vChildren[i]->WriteToStruct(childSrt);
		}
	}

	if (MVariantArray* pArray = FindWriteVariant<MVariantArray>(srt, "FixedChildren"))
	{
		uint32_t unSize = m_vFixedChildren.size();

		for (uint32_t i = 0; i < unSize; ++i)
		{
			pArray->AppendMVariant(MStruct());
			m_vFixedChildren[i]->WriteToStruct((*pArray)[i].GetVarUnsafe<MStruct>());
		}
	}
}

void MNode::ReadChildrenFromStruct(MStruct& srt)
{
	if (MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "Children"))
	{
		uint32_t unSize = pArray->GetMemberCount();
		//m_vChildren.resize(unSize);
		for (uint32_t i = 0; i < unSize; ++i)
		{
			MStruct& childSrt = *(*pArray)[i].GetStruct();
			if(MNode* pChildNode = CreateNodeByVariant(m_pEngine, childSrt))
			{
				//m_vChildren[i] = pChildNode;
				AddNodeImpl(pChildNode, MENodeChildType::ENormal);
			
			}
		}
	}
	if (MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "FixedChildren"))
	{
		uint32_t unSize = pArray->GetMemberCount();
		//m_vFixedChildren.resize(unSize);
		for (uint32_t i = 0; i < unSize; ++i)
		{
			MStruct& childSrt = *(*pArray)[i].GetStruct();
			if (MNode* pChildNode = CreateNodeByVariant(m_pEngine, childSrt))
			{
				//m_vFixedChildren[i] = pChildNode;
				AddNodeImpl(pChildNode, MENodeChildType::EFixed);
			}
		}
	}
}

void MNode::OnTick(const float& fDelta)
{

}
