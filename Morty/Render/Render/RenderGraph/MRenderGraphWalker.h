/**
 * @File         MRenderGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderTaskNode.h"
#include "TaskGraph/MTaskGraphWalker.h"

namespace morty
{

class IPropertyBlockAdapter;
class MIMesh;
class MIRenderCommand;
class MTaskNode;
class MTaskGraph;
class MRenderTaskNode;
class MORTY_API MRenderGraphWalker : public ITaskGraphWalker
{
public:
    explicit MRenderGraphWalker(const MRenderInfo& info);

    void operator()(MTaskGraph* pTaskGraph) override;


private:
    void               Render(MRenderTaskNode* pNode);

    const MRenderInfo& m_renderInfo;
};


class MORTY_API MRenderGraphSetupWalker : public ITaskGraphWalker
{
public:
    explicit MRenderGraphSetupWalker(const MRenderInfo& info);

    void operator()(MTaskGraph* pTaskGraph) override;

private:
    const MRenderInfo& m_renderInfo;
};

}// namespace morty