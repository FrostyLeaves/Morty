#include "Thread/MThreadPool.h"

#ifdef MORTY_WIN
#include <windows.h>
#include <processthreadsapi.h>

#endif


MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

std::array<METhreadType, MGlobal::M_MAX_THREAD_NUM> MThreadPool::s_tThreadType = {};

thread_local static size_t ThreadIndex = MGlobal::M_MAX_THREAD_NUM;

constexpr bool bSingleThreadMode = false;

void MThreadPool::Initialize()
{
	ThreadIndex = 0;
	s_tThreadType[ThreadIndex] = METhreadType::EMainThread;

	if (bSingleThreadMode)
	{
		m_bInitialized = true;
		return;
	}

	for (size_t nThreadIdx = 1; nThreadIdx < m_aThread.size(); ++nThreadIdx)
	{
		MString strThreadName = MString("Thread ") + MStringUtil::ToString(nThreadIdx);
		m_aThread[nThreadIdx] = std::thread(&MThreadPool::ThreadRun, this, nThreadIdx, strThreadName);
		s_tThreadType[nThreadIdx] = nThreadIdx < static_cast<int>(METhreadType::ENameThreadNum) ? static_cast<METhreadType>(nThreadIdx) : METhreadType::EAny;
		m_aThread[nThreadIdx].detach();
	}

	m_bInitialized = true;
}

void MThreadPool::Release()
{
	m_bClose = true;

	if (bSingleThreadMode)
	{
		return;
	}

	m_ConditionVariable.notify_all();
	
	//zero is MainThread.
	while (m_nCloseThreadCount != m_aThread.size() - 1);
}

bool MThreadPool::AddWork(const MThreadWork& work)
{
	if (!m_bInitialized)
	{
		MORTY_ASSERT(m_bInitialized);
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
		m_vSpecificWaitingWork[work.eThreadType].push(work);
		m_ConditionVariable.notify_all();
	}
	else
	{
		std::unique_lock<std::mutex> lck(m_ConditionMutex);
		m_vWaitingWork.push(work);
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
				if (m_bClose)
					return true;

				if (m_vSpecificWaitingWork[nThreadIndex].empty() && m_vWaitingWork.empty())
					return false;

				return true;
			});

			if (m_bClose)
			{
				++m_nCloseThreadCount;
				return;
			}

			if (!m_vSpecificWaitingWork[nThreadIndex].empty())
			{
				work = m_vSpecificWaitingWork[nThreadIndex].front();
				m_vSpecificWaitingWork[nThreadIndex].pop();
			}
            else if (!m_vWaitingWork.empty())
            {
				work = m_vWaitingWork.front();
				m_vWaitingWork.pop();
            }
		}

		if (work.funcWorkFunction)
		{
			work.funcWorkFunction();
			work = {};
		}
	}
}

std::thread::id MThreadPool::GetCurrentThreadID()
{
	return std::this_thread::get_id();
}

size_t MThreadPool::GetCurrentThreadIndex()
{
	return ThreadIndex;
}

METhreadType MThreadPool::GetCurrentThreadType()
{
	const auto id = GetCurrentThreadIndex();
	MORTY_ASSERT(id < s_tThreadType.size());
	return s_tThreadType[id];
}


