#pragma once

#include "Main/BaseWidget.h"

#include "Scene/MEntity.h"

class MScene;
class MObject;
class MEntity;
class NodeTreeView : public BaseWidget
{
public:
	NodeTreeView();
    ~NodeTreeView() = default;
	
	void Render() override;

	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;

	void Input(MInputEvent* pEvent) override;

protected:

	void RenderNode(MEntity* pNode);
	
};
