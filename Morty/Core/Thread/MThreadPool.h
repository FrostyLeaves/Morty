/**
 * @File         MThreadPool
 * 
 * @Created      2021-07-08 18:15:33
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"

#include "Thread/MThreadWork.h"

class MORTY_API MThreadPool : public MTypeClass
{
    MORTY_CLASS(MThreadPool)

public:
    MThreadPool();
    virtual ~MThreadPool();

public:

	void Initialize();
	void Release();

	bool AddWork(const MThreadWork& work);

	void ThreadRun(METhreadType eType, size_t nThreadIndex, MString strThreadName);

	static std::thread::id GetCurrentThreadID();

	static size_t GetCurrentThreadIndex();
	static METhreadType GetCurrentThreadType();

private:
	std::mutex m_ConditionMutex;

	std::condition_variable m_ConditionVariable;

	std::array<std::thread, MGlobal::M_MAX_THREAD_NUM> m_aThread;

	static std::unordered_map<size_t, METhreadType> s_tThreadType;

	std::queue<MThreadWork> m_vWaitingWork;

	std::queue<MThreadWork> m_vWaitingWorkForRender;

	bool m_bInitialized = false;
	bool m_bClose = false;
	size_t m_nCloseThreadCount = 0;
};
