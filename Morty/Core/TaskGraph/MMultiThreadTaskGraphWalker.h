/**
 * @File         MMultiThreadTaskGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MTaskGraphWalker.h"
#include "Thread/MThreadWork.h"

namespace morty
{

class MThreadPool;
class MORTY_API MMultiThreadTaskGraphWalker : public ITaskGraphWalker
{
public:
    enum class METaskState
    {
        Wait,
        Active,
        Finish,
    };

public:
    MMultiThreadTaskGraphWalker(MThreadPool* pThreadPool);

    ~MMultiThreadTaskGraphWalker();

    void operator()(MTaskGraph* pTaskGraph) override;

private:
    bool        CheckNodeActive(MTaskNode* pNode) const;

    MThreadWork CreateThreadWork(MTaskNode* pNode);

    void        OnTaskFinishedCallback(MTaskNode* pNode);

    void        ExecuteTaskNode(MTaskNode* pNode);

private:
    MThreadPool*                      m_threadPool = nullptr;

    std::queue<MTaskNode*>            m_waitTask;
    std::map<MTaskNode*, METaskState> m_nodeState;

    std::atomic_int                   m_activeTaskNum = 0;

    std::mutex                        m_taskStatehMutex;
};

}// namespace morty