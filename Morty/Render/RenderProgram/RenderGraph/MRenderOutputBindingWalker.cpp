#include "MRenderOutputBindingWalker.h"

#include "MRenderTaskNodeInput.h"
#include "Basic/MTexture.h"
#include "Render/MRenderCommand.h"
#include "Render/MRenderPass.h"
#include "Material/MMaterial.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderGraph/MRenderTaskNode.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"

void MRenderOutputBindingWalker::operator ()(MTaskGraph* pTaskGraph)
{
	MORTY_ASSERT(pTaskGraph->NeedCompile());

	std::vector<MTaskNode*> vAllNodes = pTaskGraph->GetAllNodes();

	for (MTaskNode* pNode : vAllNodes)
	{
		auto pRenderTaskNode = pNode->DynamicCast<MRenderTaskNode>();
	    for (size_t nOutputIdx = 0; nOutputIdx < pRenderTaskNode->GetOutputSize(); ++nOutputIdx)
	    {
			auto pOutput = pRenderTaskNode->GetRenderOutput(nOutputIdx);
			MORTY_ASSERT(m_tOutputs.find(pOutput->GetName()) == m_tOutputs.end());

			m_tOutputs[pOutput->GetName()] = pOutput;
	    }
	}

	for (MTaskNode* pNode : vAllNodes)
	{
		auto pRenderTaskNode = pNode->DynamicCast<MRenderTaskNode>();
		for (size_t nInputIdx = 0; nInputIdx < pRenderTaskNode->GetInputSize(); ++nInputIdx)
		{
			auto pInput = pRenderTaskNode->GetInput(nInputIdx)->DynamicCast<MRenderTaskNodeInput>();
			MORTY_ASSERT(m_tOutputs.find(pInput->GetName()) != m_tOutputs.end());

			pInput->LinkTo(m_tOutputs[pInput->GetName()]);
		}
	}

	MORTY_ASSERT(pTaskGraph->Compile());
}

