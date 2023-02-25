#include "Thread/MThreadPool.h"

#ifdef MORTY_WIN
#include <windows.h>
#include <processthreadsapi.h>

#endif


MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

std::map<std::thread::id, METhreadType> MThreadPool::s_tThreadType = {};

constexpr bool bSingleThreadMode = true;

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
	s_tThreadType[std::this_thread::get_id()] = METhreadType::EMainThread;

	if (bSingleThreadMode)
	{
		return;
	}

	// Render Thread
	size_t nRenderThreadIndex = size_t(METhreadType::ERenderThread);
	m_aThread[nRenderThreadIndex] = std::thread(&MThreadPool::ThreadRun, this, METhreadType::ERenderThread, "Thread Render");
	s_tThreadType[m_aThread[nRenderThreadIndex].get_id()] = METhreadType::ERenderThread;
	m_aThread[nRenderThreadIndex].detach();

	for (size_t i = size_t(METhreadType::ENameThreadNum); i < m_aThread.size(); ++i)
	{
		MString strThreadName = MString("Thread ") + MStringHelper::ToString(i);
		m_aThread[i] = std::thread(&MThreadPool::ThreadRun, this, METhreadType::EAny, strThreadName);
		s_tThreadType[m_aThread[i].get_id()] = METhreadType::EAny;
		m_aThread[i].detach();
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
	
	while (m_nCloseThreadCount != m_aThread.size());
}

bool MThreadPool::AddWork(const MThreadWork& work)
{
	if (!m_bInitialized)
		return false;

	if (bSingleThreadMode)
	{
		work.funcWorkFunction();
		return true;
	}
	
	if (work.eThreadType == METhreadType::ECurrentThread)
	{
		work.funcWorkFunction();
	}
	else if (work.eThreadType != METhreadType::EAny && work.eThreadType == GetCurrentThreadType())
	{
		work.funcWorkFunction();
	}
	else if (work.eThreadType == METhreadType::ERenderThread)
	{
		std::unique_lock<std::mutex> lck(m_ConditionMutex);
		m_vWaitingWorkForRender.push(work);
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

void MThreadPool::ThreadRun(METhreadType eType, MString strThreadName)
{
#ifdef MORTY_WIN
	std::wstring wstrThreadName;
	MStringHelper::ConvertToWString(strThreadName, wstrThreadName);
	SetThreadDescription(GetCurrentThread(), wstrThreadName.c_str());
#endif

	std::queue<MThreadWork>* vWorkQueue = nullptr;
	if (eType == METhreadType::ERenderThread)
	{
		vWorkQueue = &m_vWaitingWorkForRender;
	}
	else
	{
		vWorkQueue = &m_vWaitingWork;
	}

	while (true)
	{
		MThreadWork work;

		{
			std::unique_lock<std::mutex> lock(m_ConditionMutex);

			m_ConditionVariable.wait(lock, [=] {
				if (m_bClose)
					return true;

				if (vWorkQueue->empty())
					return false;

				return true;
			});

			if (m_bClose)
			{
				++m_nCloseThreadCount;
				return;
			}

			work = vWorkQueue->front();
			vWorkQueue->pop();
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
	auto id = GetCurrentThreadID();
	MORTY_ASSERT(s_tThreadType.find(id) != s_tThreadType.end());
	return s_tThreadType[id];
}

