#include "MResourceAsyncLoadSystem.h"

#include "Engine/MEngine.h"
#include "Resource/MResourceLoader.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadWork.h"
#include "Utility/MFunction.h"

using namespace morty;

void MResourceAsyncLoadSystem::Initialize() {}

void MResourceAsyncLoadSystem::AddLoader(std::shared_ptr<MResourceLoader>& pLoader)
{
    m_pendingLoader.push_back(std::move(pLoader));
}

void MResourceAsyncLoadSystem::EngineTick(const float& fDelta)
{
    MORTY_UNUSED(fDelta);

    if (m_loadWork.has_value()) { return; }

    if (!m_finishedLoader.empty())
    {
        MainThreadLoad(m_finishedLoader);
        m_finishedLoader.clear();
    }

    if (m_pendingLoader.empty()) { return; }

    constexpr size_t                            nResourceMaxLoadNum = 20;

    std::list<std::shared_ptr<MResourceLoader>> vLoader;

    if (m_pendingLoader.size() < 20) { std::swap(vLoader, m_pendingLoader); }
    else
    {
        vLoader.splice(
                vLoader.begin(),
                m_pendingLoader,
                m_pendingLoader.begin(),
                std::next(m_pendingLoader.begin(), nResourceMaxLoadNum)
        );
    }

    MThreadPool* pThreadPool = GetEngine()->GetThreadPool();

    m_loadWork                          = MThreadWork(METhreadType::EAny);
    m_loadWork.value().funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(
            MResourceAsyncLoadSystem::AnyThreadLoad,
            this,
            vLoader
    );
    pThreadPool->AddWork(m_loadWork.value());
}

void MResourceAsyncLoadSystem::AnyThreadLoad(
        const std::list<std::shared_ptr<MResourceLoader>>& vLoader
)
{
    for (auto& pLoader: vLoader)
    {
        pLoader->pResourceData = pLoader->LoadResource(pLoader->strResourceFullPath);
        if (!pLoader->pResourceData)
        {
            GetEngine()->GetLogger()->Error(
                    "Load Resource try to find: [path: {}]",
                    pLoader->strResourcePath.c_str()
            );
        }
    }

    m_finishedLoader.insert(
            m_finishedLoader.end(),
            std::make_move_iterator(vLoader.begin()),
            std::make_move_iterator(vLoader.end())
    );

    m_loadWork.reset();
}

void MResourceAsyncLoadSystem::MainThreadLoad(
        const std::list<std::shared_ptr<MResourceLoader>>& vLoader
)
{
    for (auto& pLoader: vLoader)
    {
        if (!pLoader->pResource->Load(std::move(pLoader->pResourceData)))
        {
            GetEngine()->GetLogger()->Error(
                    "Load Resource failed: [path: {}]",
                    pLoader->strResourcePath.c_str()
            );
        }
        else { pLoader->pResource->OnReload(); }
    }
}