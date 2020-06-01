/**
 * @File         MNode
 * 
 * @Created      2019-05-25 19:54:42
 *
 * @Author       Pobrecito
**/

#ifndef _M_MNODE_H_
#define _M_MNODE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MString.h"
#include "MVariant.h"
#include "MResource.h"
#include "MSerializer.h"

#include <vector>
#include <functional>

class MScene;
class MORTY_CLASS MNode : public MObject, public MSerializer
{
public:
	M_OBJECT(MNode);
    MNode();
    virtual ~MNode();	//Release memory

	enum MENodeChildType
	{
		ENormal = 0,
		EFixed = 1,
	};

	typedef std::function<bool(MNode*)> SearchNodeFunction;

public:

	virtual void SetVisible(const bool& bVisible);
	bool GetVisible() { return m_bVisible; }
	bool GetVisibleRecursively() { return m_bVisibleRecursively; }

public:

	virtual MNode* GetParent() { return m_pParent; }
	std::vector<MNode*>& GetChildren(){ return m_vChildren; }
	std::vector<MNode*>& GetFixedChildren() { return m_vFixedChildren; }
	MNode* GetRootNode();

	MNode* FindFirstChildByName(const MString& strName);
	std::vector<MNode*> FindChildrenByName(const MString& strName);
	std::vector<MNode*> FindChildrenByFunc(const SearchNodeFunction& func);

	void SetName(const MString& strName) { m_strName = strName; }
	MString GetName(){ return m_strName; }

	MScene* GetScene(){ return m_pScene; }

	//Is Holder of pNode?
	bool isHolderOf(MNode* pNode);

	virtual void OnTick(const float& fDelta);

	bool AddNode(MNode* pNode) { return AddNodeImpl(pNode, MENodeChildType::ENormal); }
	bool RemoveNode(MNode* pNode) { return !m_bDeleteMark && RemoveNodeImpl(pNode, MENodeChildType::ENormal); }

public:
	virtual bool AddNodeImpl(MNode* pNode, const MENodeChildType& etype);
	virtual bool RemoveNodeImpl(MNode* pNode, const MENodeChildType& etype);
	void RemoveAllNodeImpl(const MENodeChildType& etype);
	virtual void Tick(const float& fDelta);


	virtual void OnDelete() override;		// unbind relation

//Serialize Begin
public:


	bool Load(MResource* pResource);

	static MNode* CreateNodeByVariant(MEngine* pEngine, MStruct& var);

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

	void WriteChildrenToStruct(MStruct& srt);
	void ReadChildrenFromStruct(MStruct& srt);

//Serialize End

public:

	template <class T>
	T* FindFirstChildByType();

	template <class T>
	T* FindChildrenByType();

protected:
	template <class T>
	void FindChildrenByType(std::vector<T*>& vNodes);
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
	std::vector<MNode*> m_vFixedChildren;

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
		if (dynamic_cast<T*>(pNode))
			return dynamic_cast<T*>(pNode);

		if (T* pFindResult = pNode->FindFirstChildByType<T>())
			return pFindResult;
	}

	return nullptr;
}

template <class T>
void MNode::FindChildrenByType(std::vector<T*>& vNodes)
{
	for (MNode* pNode : m_vChildren)
	{
		if (dynamic_cast<T*>(pNode))
			vNodes.push_back(static_cast<T*>(pNode));
		pNode->FindChildrenByType<T>(vNodes);
	}
}

template <class T>
T* MNode::FindChildrenByType()
{
	std::vector<T*> vResult;
	FindChildrenByType<T>(vResult);
	return vResult;
}

#endif
