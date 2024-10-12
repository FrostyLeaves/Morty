#include "Thread/MThreadPool.h"

#ifdef MORTY_WIN
#include <windows.h>

#include <processthreadsapi.h>
#endif

using namespace morty;

MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

std::array<METhreadType, MGlobal::M_MAX_THREAD_NUM> MThreadPool::s_tThreadType = {};

thread_local static size_t ThreadIndex = MGlobal::M_MAX_THREAD_NUM;

constexpr bool             bSingleThreadMode = false;

void                       MThreadPool::Initialize()
{
    ThreadIndex                = 0;
    s_tThreadType[ThreadIndex] = METhreadType::EMainThread;

    if (bSingleThreadMode)
    {
        m_initialized = true;
        return;
    }

    for (size_t nThreadIdx = 1; nThreadIdx < m_thread.size(); ++nThreadIdx)
    {
        MString strThreadName = MString("Thread ") + MStringUtil::ToString(nThreadIdx);
        m_thread[nThreadIdx] =
                std::thread(&MThreadPool::ThreadRun, this, nThreadIdx, strThreadName);
        s_tThreadType[nThreadIdx] =
                nThreadIdx < static_cast<int>(METhreadType::ENameThreadNum)
                        ? static_cast<METhreadType>(nThreadIdx)
                        : METhreadType::EAny;
        m_thread[nThreadIdx].detach();
    }

    m_initialized = true;
}

void MThreadPool::Release()
{
    m_close = true;

    if (bSingleThreadMode) { return; }

    m_ConditionVariable.notify_all();

    //zero is MainThread.
    while (m_closeThreadCount != m_thread.size() - 1)
        ;
}

bool MThreadPool::AddWork(const MThreadWork& work)
{
    if (!m_initialized)
    {
        MORTY_ASSERT(m_initialized);
        return false;
    }

    if (bSingleThreadMode)
    {
        work.funcWorkFunction();
        return true;
    }

    if (work.eThreadType == static_cast<int>(METhreadType::ECurrentThread))
    {
        work.funcWorkFunction();
    }
    else if (work.eThreadType == static_cast<int>(GetCurrentThreadType()))
    {
        work.funcWorkFunction();
    }
    else if (work.eThreadType != static_cast<int>(METhreadType::EAny))
    {
        std::unique_lock<std::mutex> lck(m_ConditionMutex);
        m_specificWaitingWork[work.eThreadType].push(work);
        m_ConditionVariable.notify_all();
    }
    else
    {
        std::unique_lock<std::mutex> lck(m_ConditionMutex);
        m_waitingWork.push(work);
        m_ConditionVariable.notify_all();
    }

    return true;
}

void MThreadPool::ThreadRun(size_t nThreadIndex, MString strThreadName)
{
    ThreadIndex = nThreadIndex;

#ifdef MORTY_WIN
    MORTY_UNUSED(strThreadName);
    //std::wstring wstrThreadName;
    //MStringUtil::ConvertToWString(strThreadName, wstrThreadName);
    //SetThreadDescription(GetCurrentThread(), wstrThreadName.c_str());
#else
    MORTY_UNUSED(strThreadName);
#endif


    while (true)
    {
        MThreadWork work;

        {
            std::unique_lock lock(m_ConditionMutex);
            m_ConditionVariable.wait(lock, [=] {
                if (m_close) return true;

                if (m_specificWaitingWork[nThreadIndex].empty() && m_waitingWork.empty())
                    return false;

                return true;
            });

            if (m_close)
            {
                ++m_closeThreadCount;
                return;
            }

            if (!m_specificWaitingWork[nThreadIndex].empty())
            {
                work = m_specificWaitingWork[nThreadIndex].front();
                m_specificWaitingWork[nThreadIndex].pop();
            }
            else if (!m_waitingWork.empty())
            {
                work = m_waitingWork.front();
                m_waitingWork.pop();
            }
        }

        if (work.funcWorkFunction)
        {
            work.funcWorkFunction();
            work = {};
        }
    }
}

std::thread::id MThreadPool::GetCurrentThreadID() { return std::this_thread::get_id(); }

size_t          MThreadPool::GetCurrentThreadIndex() { return ThreadIndex; }

METhreadType    MThreadPool::GetCurrentThreadType()
{
    const auto id = GetCurrentThreadIndex();
    MORTY_ASSERT(id < s_tThreadType.size());
    return s_tThreadType[id];
}
