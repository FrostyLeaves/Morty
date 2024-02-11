/**
 * @File         MSingleThreadTaskGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MTaskGraphWalker.h"
#include "Thread/MThreadWork.h"

MORTY_SPACE_BEGIN

class MThreadPool;
class MORTY_API MSingleThreadTaskGraphWalker : public ITaskGraphWalker
{
public:
    enum class METaskState
    {
        Wait,
        Active,
        Finish,
    };

public:
    MSingleThreadTaskGraphWalker(MThreadPool* pThreadPool);

    void operator ()(MTaskGraph* pTaskGraph) override;

private:

    bool CheckNodeActive(MTaskNode* pNode) const;

    MThreadWork CreateThreadWork(MTaskNode* pNode);

    void OnTaskFinishedCallback(MTaskNode* pNode);

private:

    MThreadPool* m_pThreadPool = nullptr;

	std::queue<MTaskNode*> m_vWaitTask;
    std::vector<MTaskNode*> m_vActiveTask;
    std::map<MTaskNode*, METaskState> m_tNodeState;

};

MORTY_SPACE_END