#include "MNode.h"
#include "MScene.h"
#include "MVariant.h"
#include "Json/MJson.h"
#include "MFileHelper.h"
#include "Node/MNodeResource.h"

#include "MComponent.h"

#include "MFunction.h"

#include <queue>

M_OBJECT_IMPLEMENT(MNode, MObject) 

MNode::MNode()
	: MObject()
	, m_eChildType(MENodeChildType::EPublic)
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
	for (MNode* pChild : m_vProtectedChildren)
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
			for (MNode* pChildNode : pFrontNode->GetProtectedChildren())
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
	if (!pNode)
		return false;

	if (pNode->isHolderOf(this))
		return false;

	std::vector<MNode*>& children = etype == MENodeChildType::EProtected ? m_vProtectedChildren : m_vChildren;

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

void MNode::CallRecursivelyFunction(const CallFunction& func)
{
	for (MNode* pNode : m_vChildren)
	{
		func(pNode);
		pNode->CallRecursivelyFunction(func);
	}

	for (MNode* pNode : m_vProtectedChildren)
	{
		func(pNode);
		pNode->CallRecursivelyFunction(func);
	}
}

bool MNode::RemoveNodeImpl(MNode* pNode, const MENodeChildType& etype)
{
	std::vector<MNode*>& children = etype == MENodeChildType::EProtected ? m_vProtectedChildren : m_vChildren;

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
	std::vector<MNode*>& children = etype == MENodeChildType::EProtected ? m_vProtectedChildren : m_vChildren;

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

	for (MComponent* pComponent : m_vComponents)
	{
		pComponent->Tick(fDelta);
	}

	for (MNode* pChild : m_vProtectedChildren)
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

	RemoveAllNodeImpl(MENodeChildType::EPublic);
	RemoveAllNodeImpl(MENodeChildType::EProtected);

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

MNode* MNode::CreateNodeByVariant(MEngine* pEngine, const MStruct& srt)
{
	if (const MString* pString = FindReadVariant<MString>(srt, "NodeType"))
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

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Name", GetName);
	M_SERIALIZER_WRITE_VALUE("Visible", GetVisible);

	M_SERIALIZER_END;
		
	if (MString* pString = FindWriteVariant<MString>(srt, "NodeType"))
	{
		*pString = GetTypeName();
	}

	WriteComponentToStruct(srt);

	WriteChildrenToStruct(srt);
}

void MNode::ReadFromStruct(const MStruct& srt)
{
	MSerializer::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Name", SetName, String);
	M_SERIALIZER_READ_VALUE("Visible", SetVisible, Bool);

	M_SERIALIZER_END;

	ReadComponentFromStruct(srt);

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
		uint32_t unSize = m_vProtectedChildren.size();

		for (uint32_t i = 0; i < unSize; ++i)
		{
			pArray->AppendMVariant(MStruct());
			m_vProtectedChildren[i]->WriteToStruct((*pArray)[i].GetVarUnsafe<MStruct>());
		}
	}
}

void MNode::ReadChildrenFromStruct(const MStruct& srt)
{
	if (const MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "Children"))
	{
		uint32_t unSize = pArray->GetMemberCount();
		//m_vChildren.resize(unSize);
		for (uint32_t i = 0; i < unSize; ++i)
		{
			const MStruct& childSrt = *(*pArray)[i].GetStruct();
			if(MNode* pChildNode = CreateNodeByVariant(m_pEngine, childSrt))
			{
				//m_vChildren[i] = pChildNode;
				AddNodeImpl(pChildNode, MENodeChildType::EPublic);
			
			}
		}
	}
	if (const MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "FixedChildren"))
	{
		uint32_t unSize = pArray->GetMemberCount();
		//m_vFixedChildren.resize(unSize);
		for (uint32_t i = 0; i < unSize; ++i)
		{
			const MStruct& childSrt = *(*pArray)[i].GetStruct();
			if (MNode* pChildNode = CreateNodeByVariant(m_pEngine, childSrt))
			{
				//m_vFixedChildren[i] = pChildNode;
				AddNodeImpl(pChildNode, MENodeChildType::EProtected);
			}
		}
	}
}

void MNode::WriteComponentToStruct(MStruct& srt)
{
	if (MVariantArray* pArray = FindWriteVariant<MVariantArray>(srt, "Components"))
	{
		for (MComponent* pComponent : m_vComponents)
		{
			pArray->AppendMVariant(MStruct());
			MStruct& componentSrt = pArray->Back()->GetVarUnsafe<MStruct>();
			pComponent->WriteToStruct(componentSrt);
			componentSrt.AppendMVariant("ComponentType", pComponent->GetTypeIdentifier()->m_strName);
		}
	}

}

void MNode::ReadComponentFromStruct(const MStruct& srt)
{
	if (const MVariantArray* pArray = FindReadVariant<MVariantArray>(srt, "Components"))
	{
		uint32_t unSize = pArray->GetMemberCount();
		for (uint32_t i = 0; i < unSize; ++i)
		{
			const MStruct& componentSrt = *(*pArray)[i].GetStruct();
			if(const MString* pComponentTypeStr = componentSrt.FindMember<MString>("ComponentType"))
			{
				if (MTypeIdentifierConstPointer pComponentType = MTypedClass::GetType(*pComponentTypeStr))
				{
					if (MComponent* pComponent = RegisterComponent(pComponentType))
					{
						pComponent->ReadFromStruct(componentSrt);
					}
				}
			}
		}
	}
}

void MNode::OnTick(const float& fDelta)
{

}

MComponent* MNode::RegisterComponent(MTypeIdentifierConstPointer pComponentType)
{
	if (!MTypedClass::IsType(pComponentType, MComponent::GetClassTypeIdentifier()))
		return nullptr;

	auto iter = std::lower_bound(m_vComponents.begin(), m_vComponents.end(), pComponentType, [](MComponent* a, MTypeIdentifierConstPointer b) {
		return a->GetTypeIdentifier() < b;
	});

	if (iter != m_vComponents.end() && (*iter)->GetTypeIdentifier() == pComponentType)
		return (*iter);
	

	MComponent* pCreateComponent = static_cast<MComponent*>(MTypedClass::New(pComponentType->m_strName));

	pCreateComponent->SetOwnerNode(this);
	pCreateComponent->Initialize();

	if (iter == m_vComponents.end())
	{
		m_vComponents.push_back(pCreateComponent);
	}
	else
	{
		m_vComponents.insert(iter, pCreateComponent);
	}

	if (MScene* pScene = GetScene())
	{
		pScene->AddComponent(pCreateComponent);
	}

	return pCreateComponent;
}

void MNode::UnregisterComponent(MTypeIdentifierConstPointer pComponentType)
{
	if (!MTypedClass::IsType(pComponentType, MComponent::GetClassTypeIdentifier()))
		return;

	for (auto iter = m_vComponents.begin(); iter != m_vComponents.end(); ++iter)
	{
		if ((*iter)->GetTypeIdentifier() == pComponentType)
		{
			MComponent* pFoundComponent = (*iter);
			m_vComponents.erase(iter);

			if (MScene* pScene = GetScene())
			{
				pScene->RemoveComponent(pFoundComponent);
			}

			pFoundComponent->Release();
			delete pFoundComponent;
			pFoundComponent = nullptr;

			break;
		}
	}
}

MComponent* MNode::GetComponent(MTypeIdentifierConstPointer pComponentType)
{
	if (!MTypedClass::IsType(pComponentType, MComponent::GetClassTypeIdentifier()))
		return nullptr;

	auto iter = std::lower_bound(m_vComponents.begin(), m_vComponents.end(), pComponentType, [](MComponent* a, MTypeIdentifierConstPointer b) {
		return a->GetTypeIdentifier() < b;
	});

	if (iter == m_vComponents.end())
		return nullptr;

	if((*iter)->GetTypeIdentifier() == pComponentType)
		return (*iter);

	return nullptr;
}

const std::vector<MComponent*>& MNode::GetComponents() const
{
	return m_vComponents;
}

void MNode::RegisterComponentNotify(const MString& strNotifyName, const void* pComponentType, const std::function<void()>& callback)
{
	MComponentNotifyInfo* pNotifyInfo = nullptr;

	auto findResult = m_tComponentNotify.find(strNotifyName);

	if (findResult == m_tComponentNotify.end())
	{
		pNotifyInfo = new MComponentNotifyInfo();
		m_tComponentNotify.insert(std::make_pair(strNotifyName, pNotifyInfo));
	}
	else
	{
		pNotifyInfo = findResult->second;
	}

	if (!pNotifyInfo)
		return;

	pNotifyInfo->AddNotifyFunction(pComponentType, callback);
}

void MNode::UnregisterComponentNotify(const MString& strNotifyName, const void* pComponentType)
{
	auto findResult = m_tComponentNotify.find(strNotifyName);

	if (findResult == m_tComponentNotify.end())
		return;

	MComponentNotifyInfo* pNotifyInfo = findResult->second;
	m_tComponentNotify.erase(findResult);

	if (pNotifyInfo)
	{
		delete pNotifyInfo;
		pNotifyInfo = nullptr;
	}
}

void MNode::SendComponentNotify(const MString& strSignalName)
{
	auto findResult = m_tComponentNotify.find(strSignalName);

	if (findResult == m_tComponentNotify.end())
		return;

	MComponentNotifyInfo* pNotifyInfo = findResult->second;
	if (!pNotifyInfo)
		return;

	for (auto& callfunction : pNotifyInfo->m_vComponentNotifyCallFunction)
	{
		callfunction.function();
	}
}

void MComponentNotifyInfo::AddNotifyFunction(const void* pComponentType, const std::function<void()>& callback)
{
	for (auto& cf : m_vComponentNotifyCallFunction)
	{
		if (cf.pComponentType == pComponentType)
		{
			cf.function = callback;
			return;
		}
	}

	MComponentNotifyCallFunction cf;
	cf.pComponentType = pComponentType;
	cf.function = callback;
	m_vComponentNotifyCallFunction.push_back(cf);
}

void MComponentNotifyInfo::RemoveNotifyFunction(const void* pComponentType)
{
	for (auto iter =m_vComponentNotifyCallFunction.begin(); iter != m_vComponentNotifyCallFunction.end(); ++iter)
	{
		if (iter->pComponentType == pComponentType)
		{
			m_vComponentNotifyCallFunction.erase(iter);
			return;
		}
	}
}
