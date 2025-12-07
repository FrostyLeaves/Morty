#include "MRenderGraphWalker.h"

#include "MRenderGraph.h"
#include "MRenderGraphSetting.h"
#include "Material/MMaterial.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MRenderGraphWalker::MRenderGraphWalker(const MRenderInfo& info, IRenderCommand* primaryCommand)
    : m_renderInfo(info)
    , m_primaryCommand(primaryCommand)
{}

void MRenderGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(!pTaskGraph->NeedCompile());

    std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

    for (MTaskNode* currentNode: vNodes)
    {
        auto pRenderTaskNode = static_cast<MRenderTaskNode*>(currentNode);

        if (pRenderTaskNode->IsValidRenderNode()) { pRenderTaskNode->Execute(m_renderInfo, m_primaryCommand); }
    }
}

MRenderGraphSetupWalker::MRenderGraphSetupWalker(const MRenderInfo& info)
    : m_renderInfo(info)
{}

void MRenderGraphSetupWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(!pTaskGraph->NeedCompile());

    std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

    for (MTaskNode* currentNode: vNodes) { currentNode->DynamicCast<MRenderTaskNode>()->RenderSetup(m_renderInfo); }

    pTaskGraph->DynamicCast<MRenderGraph>()->GetRenderGraphSetting()->FlushDirty();
}
