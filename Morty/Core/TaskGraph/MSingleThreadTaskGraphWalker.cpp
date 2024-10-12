#include "MSingleThreadTaskGraphWalker.h"

#include "Engine/MEngine.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadPool.h"
#include "Utility/MFunction.h"

using namespace morty;

MSingleThreadTaskGraphWalker::MSingleThreadTaskGraphWalker(MThreadPool* pThreadPool)
    : m_threadPool(pThreadPool)
{}

void MSingleThreadTaskGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
    if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile()) { return; }

    std::vector<MTaskNode*> vNodes = pTaskGraph->GetOrderedNodes();

    for (MTaskNode* pCurrentNode: vNodes) { pCurrentNode->Run(); }
}
