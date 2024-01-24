#include "MRenderGraphWalker.h"

#include "Material/MMaterial.h"
#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/RenderWork/MRenderWork.h"
#include "TaskGraph/MTaskGraph.h"

MRenderGraphWalker::MRenderGraphWalker(const MRenderInfo& info)
    : m_renderInfo(info)
{
}

void MRenderGraphWalker::operator ()(MTaskGraph* pTaskGraph)
{
	MORTY_ASSERT(!pTaskGraph->NeedCompile());

	std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

	for (MTaskNode* pCurrentNode : vNodes)
	{
		auto pRenderTaskNode = pCurrentNode->DynamicCast<MRenderTaskNode>();
		pRenderTaskNode->Render(m_renderInfo);
	}
	
}

void MRenderGraphWalker::Render(MRenderTaskNode* pNode)
{
	pNode->Render(m_renderInfo);
}

MRenderGraphSetupWalker::MRenderGraphSetupWalker(const MRenderInfo& info)
	: m_renderInfo(info)
{
}

void MRenderGraphSetupWalker::operator()(MTaskGraph* pTaskGraph)
{
	MORTY_ASSERT(!pTaskGraph->NeedCompile());

	std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

	for (MTaskNode* pCurrentNode : vNodes)
	{
		pCurrentNode->DynamicCast<MRenderTaskNode>()->RenderSetup(m_renderInfo);
	}
}
