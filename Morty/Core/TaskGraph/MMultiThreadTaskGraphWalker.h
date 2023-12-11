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

    void operator ()(MTaskGraph* pTaskGraph) override;

private:

    bool CheckNodeActive(MTaskNode* pNode) const;

    MThreadWork CreateThreadWork(MTaskNode* pNode);

    void OnTaskFinishedCallback(MTaskNode* pNode);

    void ExecuteTaskNode(MTaskNode* pNode);

private:

    MThreadPool* m_pThreadPool = nullptr;

	std::queue<MTaskNode*> m_vWaitTask;
    std::map<MTaskNode*, METaskState> m_tNodeState;

    std::atomic_int m_nActiveTaskNum = 0;

    std::mutex m_taskStatehMutex;
};
