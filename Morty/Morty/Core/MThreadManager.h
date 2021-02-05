/**
 * @File         MThreadManager
 * 
 * @Created      2021-02-04 14:45:24
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTHREADMANAGER_H_
#define _M_MTHREADMANAGER_H_
#include "MGlobal.h"

#include <array>
#include <queue>
#include <mutex>
#include <thread>
#include <functional>

#define M_MAX_THREAD_NUM 10
#define M_MAX_WORK_NUM 64

struct MORTY_API MThreadWork
{
    bool bFinished;

    std::vector<size_t> vInputWorks;
    std::vector<size_t> vOutputWorks;

    std::function<void(void)> funcWorkFunction;
};

class MORTY_API MThreadManager
{
public:
    MThreadManager();
    virtual ~MThreadManager();

public:

    void InitializeThread();

    void Tick();

    size_t AddWork(MThreadWork* work);


protected:


    void ThreadRun(size_t nThreadIdx);

private:

    std::queue<MThreadWork*> m_vWaitingWorks;//等待队列
    std::queue<MThreadWork*> m_vFinishedWorks;//完成队列

    std::mutex m_ConditionMutex;
    std::condition_variable m_ConditionVariable;

    struct MSubThreadWorkDesc {
        std::thread thread;
        size_t nWorkIdx;
        std::condition_variable m_aFinishedCondVar;
    };

    std::array<std::thread, M_MAX_THREAD_NUM> m_aThread;
   
};


#endif
