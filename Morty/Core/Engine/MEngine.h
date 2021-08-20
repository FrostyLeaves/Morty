/**
 * @File         MEngine
 * 
 * @Created      2019-05-11 23:19:14
 *
 * @Author       DoubleYe
**/

#ifndef _M_MENGINE_H_
#define _M_MENGINE_H_
#include "MGlobal.h"
#include "MLogger.h"
#include "MThreadPool.h"

#include <vector>

class MISystem;
class MTaskGraph;
class MORTY_API MEngine : public MTypeClass
{
public:
	MORTY_CLASS(MEngine)

public:

	enum class EngineStage
	{
		DEFAULT,
		READY,
		RUNNING,
		STOP
	};

	struct TickTimeData
	{
		int nMaxFPS;
		float fTickInterval;
		float fTimeDelta;
		long long lPrevTickTime;
		TickTimeData(const int& nFps);

	};

public:
    MEngine();
    virtual ~MEngine();

public:

	float GetFPS() { return 1.0f / m_time.fTimeDelta; }
	float getTickDelta() { return m_time.fTimeDelta; }

public:

	MLogger* GetLogger() { return &m_logger; }
	MThreadPool* GetThreadPool() { return &m_threadPool; }
	MTaskGraph* GetMainGraph() { return m_pMainTaskGraph; }
public:

	virtual bool Initialize();
	virtual void Release();

	void Start();
	void Stop();

	void Update();


public:

	template<typename TYPE>
	TYPE* RegisterSystem();
	MISystem* FindSystem(const MType* type);

	template<typename TYPE>
	TYPE* FindSystem();

	std::vector<MISystem*>& GetAllSystem() { return m_vSystem; }

protected:

	void RegisterSystem(MISystem* pSystem);

	void Tick(const float& fDelta);

private:

	TickTimeData m_time;
	EngineStage m_eStage;


	std::map<const MType*, int> m_tSystem;
	std::vector<MISystem*> m_vSystem;

	MTaskGraph* m_pMainTaskGraph;

	MLogger m_logger;


	MThreadPool m_threadPool;
};

template<typename TYPE>
TYPE* MEngine::FindSystem()
{
	return FindSystem(TYPE::GetClassType())->DynamicCast<TYPE>();
}

template<typename TYPE>
TYPE* MEngine::RegisterSystem()
{
	if (MTypeClass::IsType<TYPE, MISystem>())
	{
		TYPE* pSystem = new TYPE();
		RegisterSystem(pSystem);
		return pSystem;
	}

	return nullptr;
}

#endif
