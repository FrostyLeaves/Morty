#include "Engine/MEngine.h"
#include "Engine/MSystem.h"
#include "Utility/MTimer.h"

#include "System/MObjectSystem.h"
#include "TaskGraph/MTaskGraph.h"

#include "Module/MCoreModule.h"
#include "TaskGraph/MMultiThreadTaskGraphWalker.h"
#include "TaskGraph/MSingleThreadTaskGraphWalker.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEngine, MTypeClass)

MEngine::MEngine()
    : MTypeClass()
    , m_time(60)
    , m_stage(EngineStage::DEFAULT)
{}

MEngine::~MEngine() = default;

bool MEngine::Initialize()
{
    m_threadPool.Initialize();
    m_mainTaskGraph = new MTaskGraph();

    m_stage = EngineStage::READY;
    return true;
}

void MEngine::Release()
{
    //wait for all thread task finished.
    m_threadPool.Release();

    for (auto& pSystem: m_systemArray)
    {
        pSystem->Release();
        delete pSystem;
        pSystem = nullptr;
    }

    m_systemTable.clear();
    m_systemArray.clear();

    delete m_mainTaskGraph;
    m_mainTaskGraph = nullptr;
}

void MEngine::Start()
{
    if (m_stage == EngineStage::READY) m_stage = EngineStage::RUNNING;
}

void MEngine::Stop() { m_stage = EngineStage::STOP; }

void MEngine::Update()
{
    if (EngineStage::RUNNING != m_stage) return;

    long long currentTime = MTimer::GetCurTime();
    if (0 == m_time.lPrevTickTime) { m_time.lPrevTickTime = currentTime; }

    float fTimeDelta = (float) (currentTime - m_time.lPrevTickTime) / 1000;

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
    for (MISystem* pSystem: m_systemArray) { pSystem->EngineTick(fDelta); }

    if (m_mainTaskGraph)
    {
        MMultiThreadTaskGraphWalker walker(&m_threadPool);
        m_mainTaskGraph->Run(&walker);
    }
}

void MEngine::RegisterSystem(MISystem* pSystem)
{
    if (FindSystem(pSystem->GetType()))
    {
        MORTY_ASSERT(!FindSystem(pSystem->GetType()));
        return;
    }

    pSystem->SetEngine(this);
    pSystem->Initialize();
    m_systemArray.push_back(pSystem);
    m_systemTable[pSystem->GetType()] = m_systemArray.size() - 1;
}

void MEngine::RegisterGlobalObject(const MType* type)
{
    if (FindGlobalObject(type)) return;

    MObject* pObject     = FindSystem<MObjectSystem>()->CreateObject(type->m_strName);
    m_globalObject[type] = pObject;
}

MISystem* MEngine::FindSystem(const MType* type)
{
    auto find = m_systemTable.find(type);
    if (find != m_systemTable.end()) { return m_systemArray[find->second]; }
    return nullptr;
}

MObject* MEngine::FindGlobalObject(const MType* type)
{
    auto find = m_globalObject.find(type);
    if (find != m_globalObject.end()) { return find->second; }
    return nullptr;
}

MEngine::TickTimeData::TickTimeData(const int& nFps)
{
    nMaxFPS       = nFps;
    fTickInterval = 1.0f / nFps;
    fTimeDelta    = 0.0f;
    lPrevTickTime = 0;
}
