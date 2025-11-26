/**
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

    ~MEngine() override;

public:
    [[nodiscard]] float GetFPS() const { return 1.0f / m_time.fTimeDelta; }

    [[nodiscard]] float getTickDelta() const { return m_time.fTimeDelta; }

public:
    MLogger*     GetLogger() { return m_logger; }

    MThreadPool* GetThreadPool() { return &m_threadPool; }

    MTaskGraph*  GetMainGraph() const { return m_mainTaskGraph; }

public:
    virtual bool Initialize();

    virtual void Release();

    void         Start();

    void         Stop();

    void         Update();

public:
    template<typename TYPE> TYPE*               RegisterSystem();

    template<typename TYPE> TYPE*               FindSystem();

    template<typename TYPE> std::weak_ptr<TYPE> FindSystemWeak();
    std::shared_ptr<MISystem>                   FindSystemShared(const MType* type);

    MISystem*                                   FindSystem(const MType* type);

    std::vector<std::shared_ptr<MISystem>>&     GetAllSystem() { return m_systemArray; }


    template<typename TYPE> TYPE*               RegisterGlobalObject();

    template<typename TYPE> TYPE*               FindGlobalObject();

    MObject*                                    FindGlobalObject(const MType* type);

protected:
    void RegisterSystem(const std::shared_ptr<MISystem>& system);

    void RegisterGlobalObject(const MType* type);

    void Tick(const float& fDelta);

private:
    TickTimeData                           m_time;
    EngineStage                            m_stage;


    std::map<const MType*, size_t>         m_systemTable;
    std::vector<std::shared_ptr<MISystem>> m_systemArray;
    std::set<const MType*>                 m_subSystemType;

    std::map<const MType*, MObject*>       m_globalObject;

    MTaskGraph*                            m_mainTaskGraph = nullptr;

    MLogger*                               m_logger;


    MThreadPool                            m_threadPool;
};

template<typename TYPE> TYPE* MEngine::FindSystem()
{
    if (auto system = FindSystem(TYPE::GetClassType())) { return system->template DynamicCast<TYPE>(); }

    return nullptr;
}

template<typename TYPE> std::weak_ptr<TYPE> MEngine::FindSystemWeak()
{
    if (auto system = FindSystemShared(TYPE::GetClassType())) { return MTypeClass::DynamicCast<TYPE>(system); }

    return {};
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

        auto system = std::make_shared<TYPE>();
        RegisterSystem(system);
        return system.get();
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