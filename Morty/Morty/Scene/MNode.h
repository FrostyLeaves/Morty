/**
 * @File         MNode
 * 
 * @Created      2019-05-25 19:54:42
 *
 * @Author       Morty
**/

#ifndef _M_MNODE_H_
#define _M_MNODE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MString.h"

#include <vector>
#include <functional>

class MIScene;
class MORTY_CLASS MNode : public MObject
{
public:
	M_OBJECT(MNode);
    MNode();
    virtual ~MNode();

	typedef std::function<bool(MNode*)> SearchNodeFunction;

public:


	void SetVisible(const bool& bVisible);
	bool GetVisible() { return m_bVisible; }
	bool GetVisibleRecursively() { return m_bVisibleRecursively; }

public:

	virtual MNode* GetParent() { return m_pParent; }
	std::vector<MNode*>& GetChildren(){ return m_vChildren; }
	MNode* GetRootNode();

	MNode* FindFirstChildByName(const MString& strName);
	std::vector<MNode*> FindChildrenByName(const MString& strName);
	std::vector<MNode*> FindChildrenByFunc(const SearchNodeFunction& func);

	virtual bool AddNode(MNode* pNode);
	virtual bool RemoveNode(MNode* pNode);

	void SetName(const MString& strName) { m_strName = strName; }
	MString GetName(){ return m_strName; }

	MIScene* GetScene(){ return m_pScene; }

	//Is Holder of pNode?
	bool isHolderOf(MNode* pNode);


	void Tick(const float& fDelta);

	virtual void OnTick(const float& fDelta);
	virtual void Render();

public:

	template <class T>
	T* FindFirstChildByType()
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
	T* FindChildrenByType()
	{
		std::vector<T*> vResult;
		FindChildrenByType<T>(vResult);
		return vResult;
	}

protected:
	template <class T>
	void FindChildrenByType(std::vector<T*>& vNodes)
	{
		for (MNode* pNode : m_vChildren)
		{
			if (dynamic_cast<T*>(pNode))
				vNodes.push_back(static_cast<T*>(pNode));
			pNode->FindChildrenByType<T>(vNodes);
		}
	}
	void FindChildrenByName(const MString& strName, std::vector<MNode*>& vNodes);
	void FindChildrenByFunc(const SearchNodeFunction& func, std::vector<MNode*>& vNodes);

	friend class MIScene;
	void SetAttachedScene(MIScene* pScene);

	void UpdateVisibleRecursively();

protected:

	MNode* m_pParent;
	MIScene* m_pScene;
	std::vector<MNode*> m_vChildren;;

	MString m_strName;

	bool m_bVisibleRecursively;
	bool m_bVisible;
};


#endif
