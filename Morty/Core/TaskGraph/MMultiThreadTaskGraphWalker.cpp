#include "MMultiThreadTaskGraphWalker.h"

#include "Engine/MEngine.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadPool.h"
#include "Utility/MFunction.h"

using namespace morty;

MMultiThreadTaskGraphWalker::MMultiThreadTaskGraphWalker(MThreadPool* pThreadPool)
    : m_threadPool(pThreadPool)
{}

MMultiThreadTaskGraphWalker::~MMultiThreadTaskGraphWalker() {}

void MMultiThreadTaskGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
    MORTY_ASSERT(m_threadPool);

    if (!m_threadPool) { return; }

    if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile()) { return; }
    const std::vector<MTaskNode*>& vNodes = pTaskGraph->GetStartNodes();

    for (MTaskNode* node: vNodes)
    {
        m_nodeState[node] = METaskState::Active;
        m_waitTask.push(node);
    }

    while (true)
    {

        {

            std::queue<MTaskNode*> vWaitTask;
            {
                std::lock_guard<std::mutex> lck(m_taskStatehMutex);
                std::swap(vWaitTask, m_waitTask);

                m_activeTaskNum += vWaitTask.size();
            }

            if (vWaitTask.empty() && m_activeTaskNum == 0) break;

            while (!vWaitTask.empty())
            {
                auto pTaskNode = vWaitTask.front();
                vWaitTask.pop();

                m_nodeState[pTaskNode] = METaskState::Active;
                m_threadPool->AddWork(CreateThreadWork(pTaskNode));
            }
        }
    }
}

bool MMultiThreadTaskGraphWalker::CheckNodeActive(MTaskNode* node) const
{
    const auto findState = m_nodeState.find(node);
    if (findState != m_nodeState.end() && findState->second != METaskState::Wait) return false;

    for (size_t nInputIdx = 0; nInputIdx < node->GetInputSize(); ++nInputIdx)
    {
        MTaskNodeInput* input = node->GetInput(nInputIdx);

        if (MTaskNode* pDependNode = input->GetLinkedNode())
        {
            const auto findDependState = m_nodeState.find(pDependNode);
            if (findDependState == m_nodeState.end() || findDependState->second != METaskState::Finish)
            {
                return false;
            }
        }
    }

    return true;
}

MThreadWork MMultiThreadTaskGraphWalker::CreateThreadWork(MTaskNode* pTaskNode)
{
    MThreadWork work;
    work.eThreadType      = static_cast<int>(pTaskNode->GetThreadType());
    work.funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(MMultiThreadTaskGraphWalker::ExecuteTaskNode, this, pTaskNode);

    return work;
}

void MMultiThreadTaskGraphWalker::ExecuteTaskNode(MTaskNode* pTaskNode)
{
    pTaskNode->Run();
    OnTaskFinishedCallback(pTaskNode);
}

void MMultiThreadTaskGraphWalker::OnTaskFinishedCallback(MTaskNode* pTaskNode)
{
    m_nodeState[pTaskNode] = METaskState::Finish;

    std::vector<MTaskNode*> vNextWaitTask;

    for (size_t i = 0; i < pTaskNode->GetOutputSize(); ++i)
    {
        MTaskNodeOutput* output = pTaskNode->GetOutput(i);
        const auto&      inputs = output->GetLinkedInputs();
        for (MTaskNodeInput* input: inputs)
        {
            MTaskNode* pNextNode = input->GetTaskNode();

            if (CheckNodeActive(pNextNode))
            {
                m_nodeState[pNextNode] = METaskState::Active;
                vNextWaitTask.push_back(pNextNode);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lck(m_taskStatehMutex);
        for (auto pNextNode: vNextWaitTask) { m_waitTask.push(pNextNode); }

        --m_activeTaskNum;
    }
}
