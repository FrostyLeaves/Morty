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

#include <vector>

class MORTY_CLASS MNode : public MObject
{
public:
    MNode();
    virtual ~MNode();

public:

	virtual MNode* GetParent() { return m_pParent; }

	virtual bool AddNode(MNode* pNode);
	virtual bool RemoveNode(MNode* pNode);

	//Is Holder of pNode?
	bool isHolderOf(MNode* pNode);

private:

	MNode* m_pParent;

	std::vector<MNode*> m_vChildren;
};


#endif
