#include "MRenderOutputBindingWalker.h"

#include "Basic/MTexture.h"
#include "MRenderTaskNodeInput.h"
#include "Material/MMaterial.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderCommon.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

using namespace morty;

void MRenderOutputBindingWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(pTaskGraph->NeedCompile());

    std::vector<MTaskNode*> vAllNodes = pTaskGraph->GetAllNodes();

    for (MTaskNode* pNode: vAllNodes)
    {
        auto pRenderTaskNode = pNode->DynamicCast<MRenderTaskNode>();
        for (size_t nOutputIdx = 0; nOutputIdx < pRenderTaskNode->GetOutputSize(); ++nOutputIdx)
        {
            auto pOutput = pRenderTaskNode->GetRenderOutput(nOutputIdx);
            MORTY_ASSERT(m_outputs.find(pOutput->GetName()) == m_outputs.end());

            m_outputs[pOutput->GetName()] = pOutput;
        }
    }

    for (MTaskNode* pNode: vAllNodes)
    {
        auto pRenderTaskNode = pNode->DynamicCast<MRenderTaskNode>();
        for (size_t nInputIdx = 0; nInputIdx < pRenderTaskNode->GetInputSize(); ++nInputIdx)
        {
            auto pInput = pRenderTaskNode->GetInput(nInputIdx)->DynamicCast<MRenderTaskNodeInput>();
            MORTY_ASSERT(m_outputs.find(pInput->GetName()) != m_outputs.end());

            pInput->LinkTo(m_outputs[pInput->GetName()]);
        }
    }

    MORTY_ASSERT(pTaskGraph->Compile());
}
