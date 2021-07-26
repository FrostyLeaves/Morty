#ifndef _TASK_GRAPH_VIEW_H_
#define _TASK_GRAPH_VIEW_H_

#include "IBaseView.h"

class MNode;
class MObject;
class MTaskGraph;
class MTaskNodeOutput;
class TaskGraphView : public IBaseView
{
public:
	TaskGraphView();
	virtual ~TaskGraphView();


	void SetRenderGraph(MTaskGraph* pRenderGraph);

public:
	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


private:
	MEngine* m_pEngine;

	MTaskGraph* m_pRenderGraph;
	MTaskNodeOutput* m_pSelectedOutput;
};







#endif