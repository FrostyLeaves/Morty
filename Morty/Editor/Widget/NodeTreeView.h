#ifndef _NODE_TREE_VIEW_H_
#define _NODE_TREE_VIEW_H_

#include "Main/IBaseView.h"

#include "Scene/MEntity.h"

class MScene;
class MObject;
class MEntity;
class NodeTreeView : public IBaseView
{
public:
	NodeTreeView();
	virtual ~NodeTreeView();


public:
	void SetScene(MScene* pScene);

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;

protected:

	void RenderNode(MEntity* pNode);

private:
	MScene* m_pScene;

	MEngine* m_pEngine;
};







#endif