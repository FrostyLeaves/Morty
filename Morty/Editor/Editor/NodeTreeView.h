#ifndef _NODE_TREE_VIEW_H_
#define _NODE_TREE_VIEW_H_

#include "IBaseView.h"

class MObject;
class MNode;
class NodeTreeView : public IBaseView
{
public:
	NodeTreeView();
	virtual ~NodeTreeView();


public:
	void SetRootNode(MNode* pObject);

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


	MNode* GetSelectionNode();

protected:

	void RenderNode(MNode* pNode);

private:
	MObjectID m_unRootNodeID;

	MObjectID m_unSelectedObjectID;

	MEngine* m_pEngine;
};







#endif