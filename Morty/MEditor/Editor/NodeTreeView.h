#ifndef _NODE_TREE_VIEW_H_
#define _NODE_TREE_VIEW_H_

#include "MGlobal.h"

class MObject;
class MNode;
class NodeTreeView
{
public:
	NodeTreeView();
	virtual ~NodeTreeView();


public:
	void SetRootNode(MNode* pObject);

	void Render();

	MObject* GetSelectionNode();

protected:

	void RenderNode(MNode* pNode);

private:
	MNode* m_pRootNode;

	MObjectID m_unSelectedObjectID;
};







#endif