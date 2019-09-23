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

class MIScene;
class MORTY_CLASS MNode : public MObject
{
public:
    MNode();
    virtual ~MNode();

public:


	void SetVisible(const bool& bVisible);
	bool GetVisible() { return m_bVisible; }

public:

	virtual MNode* GetParent() { return m_pParent; }
	std::vector<MNode*>& GetChildren(){ return m_vChildren; }
	MNode* GetRootNode();

	MNode* FindFirstChildByName(const MString& strName);

	virtual bool AddNode(MNode* pNode);
	virtual bool RemoveNode(MNode* pNode);

	void SetName(const MString& strName) { m_strName = strName; }
	MString GetName(){ return m_strName; }

	//Is Holder of pNode?
	bool isHolderOf(MNode* pNode);


	void Tick(const float& fDelta);

	virtual void OnTick(const float& fDelta);
	virtual void Render();

protected:

	friend class MIScene;
	void SetAttachedScene(MIScene* pScene);

protected:

	MNode* m_pParent;
	MIScene* m_pScene;
	std::vector<MNode*> m_vChildren;;

	MString m_strName;

	bool m_bVisible;
};


#endif
