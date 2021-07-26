#include "MThreadPool.h"


MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

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
	for (size_t i = 0; i < m_aThread.size(); ++i)
	{
		m_aThread[i] = std::thread(&MThreadPool::ThreadRun, this, i);
		m_aThread[i].detach();
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

void MThreadPool::ThreadRun(size_t nThreadIdx)
{
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
