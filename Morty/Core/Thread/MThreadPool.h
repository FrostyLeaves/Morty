/**
 * @File         MThreadPool
 * 
 * @Created      2021-07-08 18:15:33
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTHREADPOOL_H_
#define _M_MTHREADPOOL_H_
#include "MGlobal.h"
#include "MType.h"

#include "MThreadWork.h"

#define M_MAX_THREAD_NUM 10

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

	void ThreadRun(size_t nThreadIdx, MString strThreadName);

	static std::thread::id GetCurrentThreadID();

	static METhreadType GetCurrentThreadType();

private:
	std::mutex m_ConditionMutex;
	std::condition_variable m_ConditionVariable;

	struct MSubThreadWorkDesc {
		std::thread thread;
		size_t nWorkIdx;
		std::condition_variable m_aFinishedCondVar;
	};

	std::array<std::thread, M_MAX_THREAD_NUM> m_aThread;

	static std::map<std::thread::id, METhreadType> s_tThreadType;

	std::queue<MThreadWork> m_vWaitingWork;

	bool m_bInitialized;


	bool m_bClose;
	int m_nCloseThreadCount;
};


#endif
