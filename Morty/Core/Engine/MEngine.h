﻿/**
 * @File         MEngine
 * 
 * @Created      2019-05-11 23:19:14
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Thread/MThreadPool.h"
#include "Utility/MLogger.h"

namespace morty
{

class MObject;
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

    struct TickTimeData {
        int       nMaxFPS;
        float     fTickInterval;
        float     fTimeDelta;
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
    MLogger*     GetLogger() { return &m_logger; }

    MThreadPool* GetThreadPool() { return &m_threadPool; }

    MTaskGraph*  GetMainGraph() { return m_mainTaskGraph; }

public:
    virtual bool Initialize();

    virtual void Release();

    void         Start();

    void         Stop();

    void         Update();

public:
    template<typename TYPE> TYPE* RegisterSystem();

    template<typename TYPE> TYPE* FindSystem();

    MISystem*                     FindSystem(const MType* type);

    std::vector<MISystem*>&       GetAllSystem() { return m_systemArray; }


    template<typename TYPE> TYPE* RegisterGlobalObject();

    template<typename TYPE> TYPE* FindGlobalObject();

    MObject*                      FindGlobalObject(const MType* type);

protected:
    void RegisterSystem(MISystem* pSystem);

    void RegisterGlobalObject(const MType* type);

    void Tick(const float& fDelta);

private:
    TickTimeData                     m_time;
    EngineStage                      m_stage;


    std::map<const MType*, size_t>   m_systemTable;
    std::vector<MISystem*>           m_systemArray;
    std::set<const MType*>           m_subSystemType;

    std::map<const MType*, MObject*> m_globalObject;

    MTaskGraph*                      m_mainTaskGraph = nullptr;

    MLogger                          m_logger;


    MThreadPool                      m_threadPool;
};

template<typename TYPE> TYPE* MEngine::FindSystem()
{
    auto pSystem = FindSystem(TYPE::GetClassType());
    if (pSystem) { return pSystem->template DynamicCast<TYPE>(); }

    return nullptr;
}

template<typename TYPE> TYPE* MEngine::FindGlobalObject()
{
    return FindGlobalObject(TYPE::GetClassType())->template DynamicCast<TYPE>();
}

template<typename TYPE> TYPE* MEngine::RegisterSystem()
{
    if (MTypeClass::IsType<TYPE, MISystem>())
    {
        MORTY_ASSERT(MTypeClass::GetClassType() != MISystem::GetClassType());

        TYPE* pSystem = new TYPE();
        RegisterSystem(pSystem);
        return pSystem;
    }

    return nullptr;
}

template<typename TYPE> TYPE* MEngine::RegisterGlobalObject()
{
    if (MTypeClass::IsType<TYPE, MObject>())
    {
        RegisterGlobalObject(TYPE::GetClassType());
        return FindGlobalObject(TYPE::GetClassType())->template DynamicCast<TYPE>();
    }

    return nullptr;
}

}// namespace morty