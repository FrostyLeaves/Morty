#include "MThreadManager.h"

MThreadManager::MThreadManager()
{
}

MThreadManager::~MThreadManager()
{

}

void MThreadManager::InitializeThread()
{
	for (size_t i = 0; i < m_aThread.size(); ++i)
	{
		m_aThread[i] = std::thread(&MThreadManager::ThreadRun, this, i);
		m_aThread[i].detach();
	}
}

size_t MThreadManager::AddWork(MThreadWork* work)
{
	{
		std::unique_lock <std::mutex> lck(m_ConditionMutex);

		m_vWaitingWorks.push(work);

		m_ConditionVariable.notify_one(); //唤醒一个线程.
	}

	return 0;
}

void MThreadManager::ThreadRun(size_t nThreadIdx)
{
	while (true)
	{
		MThreadWork* work = nullptr;

		{
			std::unique_lock<std::mutex> lock(m_ConditionMutex);

			// 等待主线程通知起来干活
			m_ConditionVariable.wait(lock, [=] { return !m_vWaitingWorks.empty(); });

			//拿到自己的工作
			work = m_vWaitingWorks.front();
			m_vWaitingWorks.pop();			

			m_vFinishedWorks.push(work);
		}

		//完成工作
		work->funcWorkFunction();

		work->bFinished = true;
	}
}
