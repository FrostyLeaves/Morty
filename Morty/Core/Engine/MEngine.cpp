#include "Engine/MEngine.h"
#include "Engine/MSystem.h"
#include "Utility/MTimer.h"

#include "TaskGraph/MTaskGraph.h"
#include "System/MObjectSystem.h"

#include "Module/MCoreModule.h"

MORTY_CLASS_IMPLEMENT(MEngine, MTypeClass)

MEngine::MEngine()
	: MTypeClass()
	, m_time(60)
	, m_eStage(EngineStage::DEFAULT)
	, m_pMainTaskGraph(nullptr)
{

}

MEngine::~MEngine()
{
}

bool MEngine::Initialize()
{
	m_threadPool.Initialize();

	MCoreModule::Register(this);

	m_pMainTaskGraph = FindSystem<MObjectSystem>()->CreateObject<MTaskGraph>();

	m_eStage = EngineStage::READY;
	return true;
}

void MEngine::Release()
{
	m_pMainTaskGraph->DeleteLater();
	m_pMainTaskGraph = nullptr;

	m_threadPool.Release();

	for (MISystem* pSystem : m_vSystem)
	{
		pSystem->Release();
		delete pSystem;
		pSystem = nullptr;
	}

	m_tSystem.clear();
	m_vSystem.clear();
}

void MEngine::Start()
{
	if (m_eStage == EngineStage::READY)
		m_eStage = EngineStage::RUNNING;
}

void MEngine::Stop()
{
	m_eStage = EngineStage::STOP;
}

void MEngine::Update()
{
	if (EngineStage::RUNNING != m_eStage)
		return;

	long long currentTime = MTimer::GetCurTime();
	if (0 == m_time.lPrevTickTime)
	{
		m_time.lPrevTickTime = currentTime;
	}

	float fTimeDelta = (float)(currentTime - m_time.lPrevTickTime) / 1000;

	if (fTimeDelta - m_time.fTickInterval >= 0)
	{
		m_time.fTimeDelta = fTimeDelta;

		//tick
		Tick(fTimeDelta);
		m_time.lPrevTickTime = currentTime;
	}

}

void MEngine::Tick(const float& fDelta)
{
	for (MISystem* pSystem : m_vSystem)
	{
		pSystem->EngineTick(fDelta);
	}

	if (m_pMainTaskGraph)
	{
		m_pMainTaskGraph->Run();
	}
}

void MEngine::RegisterSystem(MISystem* pSystem)
{
	if (FindSystem(pSystem->GetType()))
		return;

	pSystem->SetEngine(this);
	pSystem->Initialize();
	m_vSystem.push_back(pSystem);
	m_tSystem[pSystem->GetType()] = m_vSystem.size() - 1;
}

MISystem* MEngine::FindSystem(const MType* type)
{
	auto find = m_tSystem.find(type);
	if (find != m_tSystem.end())
		return m_vSystem[find->second];

	return nullptr;
}

MEngine::TickTimeData::TickTimeData(const int& nFps)
{
	nMaxFPS = nFps;
	fTickInterval = 1.0f / nFps;
	fTimeDelta = 0.0f;
	lPrevTickTime = 0;
}
