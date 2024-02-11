#pragma once

#include "Main/BaseWidget.h"

MORTY_SPACE_BEGIN

class MTaskGraph;
class TaskGraphView : public BaseWidget
{
public:
	TaskGraphView(const MString& viewName);
    ~TaskGraphView() = default;

	void Render() override;
	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;

	void SetTaskGraph(MTaskGraph* pGraph);

	MTaskGraph* m_pTaskGraph = nullptr;
};

MORTY_SPACE_END