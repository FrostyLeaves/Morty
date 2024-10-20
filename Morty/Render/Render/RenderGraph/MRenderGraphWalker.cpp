#include "MRenderGraphWalker.h"

#include "MRenderGraph.h"
#include "MRenderGraphSetting.h"
#include "Material/MMaterial.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

MRenderGraphWalker::MRenderGraphWalker(const MRenderInfo& info)
    : m_renderInfo(info)
{}

void MRenderGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(!pTaskGraph->NeedCompile());

    std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

    for (MTaskNode* pCurrentNode: vNodes)
    {
        bool bOutputExist    = true;
        auto pRenderTaskNode = pCurrentNode->DynamicCast<MRenderTaskNode>();
        for (size_t nOutputIdx = 0; nOutputIdx < pRenderTaskNode->GetOutputSize(); ++nOutputIdx)
        {
            bOutputExist &= (nullptr != pRenderTaskNode->GetOutputTexture(nOutputIdx));
        }

        if (bOutputExist) { pRenderTaskNode->Render(m_renderInfo); }
    }
}

void MRenderGraphWalker::Render(MRenderTaskNode* pNode) { pNode->Render(m_renderInfo); }

MRenderGraphSetupWalker::MRenderGraphSetupWalker(const MRenderInfo& info)
    : m_renderInfo(info)
{}

void MRenderGraphSetupWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(!pTaskGraph->NeedCompile());

    std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

    for (MTaskNode* pCurrentNode: vNodes) { pCurrentNode->DynamicCast<MRenderTaskNode>()->RenderSetup(m_renderInfo); }

    pTaskGraph->DynamicCast<MRenderGraph>()->GetRenderGraphSetting()->FlushDirty();
}
