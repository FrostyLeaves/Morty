#pragma once

#include "Main/BaseWidget.h"

#include "Scene/MEntity.h"

namespace morty
{

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

protected:
    void RenderNode(MEntity* pNode);
};

}// namespace morty