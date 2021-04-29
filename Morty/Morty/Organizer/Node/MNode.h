/**
 * @File         MNode
 * 
 * @Created      2019-05-25 19:54:42
 *
 * @Author       DoubleYe
**/

#ifndef _M_MNODE_H_
#define _M_MNODE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MString.h"
#include "MVariant.h"
#include "MResource.h"
#include "MComponent.h"
#include "MSerializer.h"

#include <vector>
#include <functional>

class MScene;


class MORTY_API MComponentNotifyInfo
{
public:
	struct MComponentNotifyCallFunction
	{
		const void* pComponentType;
		std::function<void()> function;

		bool operator ==(const MComponentNotifyCallFunction& cf) { return pComponentType == cf.pComponentType; }
	};
public:
	MComponentNotifyInfo() :m_vComponentNotifyCallFunction() {}

	void AddNotifyFunction(const void* pComponentType, const std::function<void()>& callback);
	void RemoveNotifyFunction(const void* pComponentType);

public:

	std::vector<MComponentNotifyCallFunction> m_vComponentNotifyCallFunction;
};

class MORTY_API MNode : public MObject, public MSerializer
{
public:
	M_OBJECT(MNode);
    MNode();
    virtual ~MNode();	//Release memory

	enum class MENodeChildType
	{
		EPublic = 1,
		EProtected = 2,
		EPrivate = 4,
	};

	typedef std::function<bool(MNode*)> SearchNodeFunction;
	typedef std::function<void(MNode*)> CallFunction;

public:

	virtual void SetVisible(const bool& bVisible);
	bool GetVisible() { return m_bVisible; }
	bool GetVisibleRecursively() { return m_bVisibleRecursively; }

public:

	virtual MNode* GetParent() { return m_pParent; }
	std::vector<MNode*>& GetChildren(){ return m_vChildren; }
	std::vector<MNode*>& GetProtectedChildren() { return m_vProtectedChildren; }
	MNode* GetRootNode();

	MNode* FindFirstChildByName(const MString& strName);
	std::vector<MNode*> FindChildrenByName(const MString& strName);
	std::vector<MNode*> FindChildrenByFunc(const SearchNodeFunction& func);

	void CallRecursivelyFunction(const CallFunction& func);

	void SetName(const MString& strName) { m_strName = strName; }
	MString GetName(){ return m_strName; }

	MScene* GetScene(){ return m_pScene; }

	//Is Holder of pNode?
	bool isHolderOf(MNode* pNode);

	virtual void OnTick(const float& fDelta);

	bool AddNode(MNode* pNode) { return AddNodeImpl(pNode, MENodeChildType::EPublic); }
	bool RemoveNode(MNode* pNode) { return !m_bDeleteMark && RemoveNodeImpl(pNode, MENodeChildType::EPublic); }


public:

	template <class T>
	T* RegisterComponent();
	MComponent* RegisterComponent(MTypeIdentifierConstPointer pComponentType);

	template <class T>
	void UnregisterComponent();
	void UnregisterComponent(MTypeIdentifierConstPointer pComponentType);

	template <class T>
	T* GetComponent();
	MComponent* GetComponent(MTypeIdentifierConstPointer pComponentType);

	std::vector<MComponent*> GetComponents();

	template <class T>
	void RegisterComponentNotify(const MString& strNotifyName, const std::function<void()>& callback);
	void RegisterComponentNotify(const MString& strNotifyName, const void* pComponentType, const std::function<void()>& callback);

	template <class T>
	void UnregisterComponentNotify(const MString& strNotifyName);
	void UnregisterComponentNotify(const MString& strNotifyName, const void* pComponentType);

	void SendComponentNotify(const MString& strSignalName);

public:

	void AddProtectedNode(MNode* pNode) { AddNodeImpl(pNode, MENodeChildType::EProtected); }

protected:
	virtual bool AddNodeImpl(MNode* pNode, const MENodeChildType& etype);
	virtual bool RemoveNodeImpl(MNode* pNode, const MENodeChildType& etype);
	virtual void ParentChangeImpl(MNode* pParent);
	
	void RemoveAllNodeImpl(const MENodeChildType& etype);
	
	
	virtual void Tick(const float& fDelta);


	virtual void OnDelete() override;		// unbind relation

//Serialize Begin
public:


	bool Load(MResource* pResource);

	static MNode* CreateNodeByVariant(MEngine* pEngine, const MStruct& var);

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(const MStruct& srt) override;

	void WriteChildrenToStruct(MStruct& srt);
	void ReadChildrenFromStruct(const MStruct& srt);

	void WriteComponentToStruct(MStruct& srt);
	void ReadComponentFromStruct(const MStruct& srt);

//Serialize End

public:

	template <class T>
	T* FindFirstChildByType();

	template <class T>
	void FindChildrenByType(std::vector<T*>& vNodes, const int& nChildType = static_cast<int>(MENodeChildType::EPublic));

	template <class T>
	T* FindParentByType();

protected:
	void FindChildrenByName(const MString& strName, std::vector<MNode*>& vNodes);
	void FindChildrenByFunc(const SearchNodeFunction& func, std::vector<MNode*>& vNodes);

	friend class MScene;
	void SetAttachedScene(MScene* pScene);

	void UpdateVisibleRecursively();

protected:
	MENodeChildType m_eChildType;
	MNode* m_pParent;
	MScene* m_pScene;
	std::vector<MNode*> m_vChildren;
	std::vector<MNode*> m_vProtectedChildren;
	std::map<const void*, MComponent*> m_tComponents;
	std::map<MString, MComponentNotifyInfo*> m_tComponentNotify;

	bool m_bVisibleRecursively;


protected:
	MString m_strName;
	bool m_bVisible;
};

template <class T>
T* MNode::FindFirstChildByType()
{
	for (MNode* pNode : m_vChildren)
	{
		if (pNode->DynamicCast<T>())
			return dynamic_cast<T*>(pNode);

		if (T* pFindResult = pNode->FindFirstChildByType<T>())
			return pFindResult;
	}

	return nullptr;
}

template <class T>
void MNode::FindChildrenByType(std::vector<T*>& vNodes, const int& nChildType)
{
	if (nChildType & static_cast<int>(MENodeChildType::EPublic))
	{
		for (MNode* pNode : m_vChildren)
		{
			if (pNode->DynamicCast<T>())
				vNodes.push_back(static_cast<T*>(pNode));
			pNode->FindChildrenByType<T>(vNodes);
		}
	}

	if (nChildType & static_cast<int>(MENodeChildType::EProtected))
	{
		for (MNode* pNode : m_vProtectedChildren)
		{
			if (pNode->DynamicCast<T>())
				vNodes.push_back(static_cast<T*>(pNode));
			pNode->FindChildrenByType<T>(vNodes, nChildType);
		}
	}
}

template <class T>
T* MNode::FindParentByType()
{
	MNode* pParent = GetParent();
	T* pTypedParent = nullptr;
	while (pParent)
	{
		if (pTypedParent = pParent->DynamicCast<T>())
			return pTypedParent;

		pParent = pParent->GetParent();
	}

	return nullptr;
}

template <class T>
T* MNode::RegisterComponent()
{
	MTypeIdentifierConstPointer pComponentType = T::GetClassTypeIdentifier();
	return static_cast<T*>(RegisterComponent(pComponentType));
}

template <class T>
void MNode::UnregisterComponent()
{
	UnregisterComponent(T::GetClassTypeIdentifier());
}

template <class T>
T* MNode::GetComponent()
{
	return static_cast<T*>(GetComponent(T::GetClassTypeIdentifier()));
}


template <class T>
void MNode::RegisterComponentNotify(const MString& strNotifyName, const std::function<void()>& callback)
{
	RegisterComponentNotify(strNotifyName, T::GetClassTypeIdentifier(), callback);
}

template <class T>
void MNode::UnregisterComponentNotify(const MString& strNotifyName)
{
	UnregisterComponentNotify(strNotifyName, T::GetClassTypeIdentifier());
}

#endif