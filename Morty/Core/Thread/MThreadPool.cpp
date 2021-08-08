#include "MThreadPool.h"

#ifdef MORTY_WIN
#include <windows.h>
#include <processthreadsapi.h>

#endif


MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

std::map<std::thread::id, METhreadType> MThreadPool::s_tThreadType = {};

MThreadPool::MThreadPool()
	: m_bInitialized(false)
	, m_bClose(false)
	, m_nCloseThreadCount(0)
{

}

MThreadPool::~MThreadPool()
{

}

void MThreadPool::Initialize()
{

	// Render Thread
	m_aThread[0] = std::thread(&MThreadPool::ThreadRun, this, 0, "Thread Render");
	m_aThread[0].detach();
	s_tThreadType[m_aThread[0].get_id()] = METhreadType::ERenderThread;

	for (size_t i = 1; i < m_aThread.size(); ++i)
	{
		MString strThreadName = MString("Thread ") + MStringHelper::ToString(i);
		m_aThread[i] = std::thread(&MThreadPool::ThreadRun, this, i, strThreadName);
		m_aThread[i].detach();
		s_tThreadType[m_aThread[i].get_id()] = METhreadType::EAny;
	}

	m_bInitialized = true;
}

void MThreadPool::Release()
{
	m_bClose = true;

	m_ConditionVariable.notify_all();
	

	while (m_nCloseThreadCount != m_aThread.size());
}

bool MThreadPool::AddWork(const MThreadWork& work)
{
	if (!m_bInitialized)
		return false;

	if (work.eThreadType == METhreadType::EMainThread)
	{
		work.funcWorkFunction();
	}
	else
	{
		std::unique_lock<std::mutex> lck(m_ConditionMutex);
		m_vWaitingWork.push(work);
		m_ConditionVariable.notify_all();
	}

	return true;
}

void MThreadPool::ThreadRun(size_t nThreadIdx, MString strThreadName)
{
#ifdef MORTY_WIN
	std::wstring wstrThreadName;
	MStringHelper::ConvertToWString(strThreadName, wstrThreadName);
	SetThreadDescription(GetCurrentThread(), wstrThreadName.c_str());
#endif

	while (true)
	{
		MThreadWork work;

		{
			std::unique_lock<std::mutex> lock(m_ConditionMutex);

			m_ConditionVariable.wait(lock, [=] {
				if (m_bClose)
					return true;

				if (m_vWaitingWork.empty())
					return false;

				auto& work = m_vWaitingWork.front();

				if (work.eThreadType != METhreadType::EAny && (int)work.eThreadType != nThreadIdx)
				{
					return false;
				}

				return true;
			});

			if (m_bClose)
			{
				++m_nCloseThreadCount;
				return;
			}

			work = m_vWaitingWork.front();
			m_vWaitingWork.pop();
		}

		work.funcWorkFunction();
	}
}

std::thread::id MThreadPool::GetCurrentThreadID()
{
	return std::this_thread::get_id();
}

METhreadType MThreadPool::GetCurrentThreadType()
{
	return s_tThreadType[GetCurrentThreadID()];
}

