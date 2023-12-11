#include "MSingleThreadTaskGraphWalker.h"

#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadPool.h"

MSingleThreadTaskGraphWalker::MSingleThreadTaskGraphWalker(MThreadPool* pThreadPool)
	: m_pThreadPool(pThreadPool)
{

}

void MSingleThreadTaskGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
	{
		return;
	}

	std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

	for (MTaskNode* pCurrentNode : vNodes)
	{
		pCurrentNode->Run();
	}
}
