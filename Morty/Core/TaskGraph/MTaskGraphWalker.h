/**
 * @File         MTaskGraphWalker
 * 
 * @Created      2021-07-08 14:46:43
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTASKGRAPHWALKER_H_
#define _M_MTASKGRAPHWALKER_H_
#include "MGlobal.h"
#include "MThreadWork.h"

class MTaskNode;
class MTaskGraph;
class MORTY_API MTaskGraphWalker
{
public:
    enum class METaskState
    {
        Wait,
        Active,
        Finish,
    };

public:
    MTaskGraphWalker();
    virtual ~MTaskGraphWalker();

public:

    void operator ()(MTaskGraph* pTaskGraph);

private:

    bool CheckNodeActive(MTaskNode* pNode);

    MThreadWork CreateThreadWork(MTaskNode* pNode);

    void OnTaskFinishedCallback(MTaskNode* pNode);

private:

	std::queue<MTaskNode*> m_vWaitTask;
    std::vector<MTaskNode*> m_vActiveTask;
    std::map<MTaskNode*, METaskState> m_tNodeState;


    std::mutex m_taskStatehMutex;
};


#endif
