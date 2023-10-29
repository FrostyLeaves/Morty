#include "Thread/MThreadPool.h"

#ifdef MORTY_WIN
#include <windows.h>
#include <processthreadsapi.h>

#endif


MORTY_CLASS_IMPLEMENT(MThreadPool, MTypeClass)

std::unordered_map<size_t, METhreadType> MThreadPool::s_tThreadType = {};

thread_local static size_t ThreadIndex = 0;

constexpr bool bSingleThreadMode = false;

MThreadPool::MThreadPool()
{

}

MThreadPool::~MThreadPool()
{

}

void MThreadPool::Initialize()
{
	ThreadIndex = 0;
	s_tThreadType[ThreadIndex] = METhreadType::EMainThread;

	if (bSingleThreadMode)
	{
		m_bInitialized = true;
		return;
	}

	// Render Thread
	const size_t nRenderThreadIndex = size_t(METhreadType::ERenderThread);
	m_aThread[nRenderThreadIndex] = std::thread(&MThreadPool::ThreadRun, this, METhreadType::ERenderThread, nRenderThreadIndex, "Thread Render");
	s_tThreadType[nRenderThreadIndex] = METhreadType::ERenderThread;
	m_aThread[nRenderThreadIndex].detach();

	for (size_t nThreadIdx = size_t(METhreadType::ENameThreadNum); nThreadIdx < m_aThread.size(); ++nThreadIdx)
	{
		MString strThreadName = MString("Thread ") + MStringUtil::ToString(nThreadIdx);
		m_aThread[nThreadIdx] = std::thread(&MThreadPool::ThreadRun, this, METhreadType::EAny, nThreadIdx, strThreadName);
		s_tThreadType[nThreadIdx] = METhreadType::EAny;
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

void MThreadPool::ThreadRun(METhreadType eType, size_t nThreadIndex, MString strThreadName)
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

size_t MThreadPool::GetCurrentThreadIndex()
{
	return ThreadIndex;
}

METhreadType MThreadPool::GetCurrentThreadType()
{
	auto id = GetCurrentThreadIndex();
	MORTY_ASSERT(s_tThreadType.find(id) != s_tThreadType.end());
	return s_tThreadType[id];
}


