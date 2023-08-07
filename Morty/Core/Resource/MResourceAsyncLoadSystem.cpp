#include "MResourceAsyncLoadSystem.h"

#include "Engine/MEngine.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadWork.h"
#include "Utility/MFunction.h"
#include "Resource/MResourceLoader.h"

void MResourceAsyncLoadSystem::Initialize()
{

}

void MResourceAsyncLoadSystem::AddLoader(std::shared_ptr<MResourceLoader>& pLoader)
{
    m_vPendingLoader.push_back(std::move(pLoader));
}

void MResourceAsyncLoadSystem::EngineTick(const float& fDelta)
{
	if (m_loadWork.has_value())
	{
		return;
	}

	if (!m_vFinishedLoader.empty())
	{
		MainThreadLoad(m_vFinishedLoader);
		m_vFinishedLoader.clear();
	}

	if (m_vPendingLoader.empty())
	{
		return;
	}

	constexpr size_t nResourceMaxLoadNum = 20;

	std::list<std::shared_ptr<MResourceLoader>> vLoader;

	if (m_vPendingLoader.size() < 20)
	{
		std::swap(vLoader, m_vPendingLoader);
	}
	else
	{
		vLoader.splice(vLoader.begin(),
			m_vPendingLoader,
			m_vPendingLoader.begin(),
			std::next(m_vPendingLoader.begin(), nResourceMaxLoadNum));
	}

	MThreadPool* pThreadPool = GetEngine()->GetThreadPool();

	m_loadWork = MThreadWork();
	m_loadWork.value().funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(MResourceAsyncLoadSystem::AnyThreadLoad, this, vLoader);
	m_loadWork.value().eThreadType = METhreadType::EAny;
	pThreadPool->AddWork(m_loadWork.value());
}

void MResourceAsyncLoadSystem::AnyThreadLoad(const std::list<std::shared_ptr<MResourceLoader>>& vLoader)
{
	for (auto& pLoader : vLoader)
	{
		pLoader->pResourceData = pLoader->LoadResource(pLoader->strResourceFullPath, pLoader->strResourcePath);
		if (!pLoader->pResourceData)
		{
			GetEngine()->GetLogger()->Error("Load Resource try to find: [path: {}]", pLoader->strResourcePath.c_str());
		}
	}

	m_vFinishedLoader.insert(m_vFinishedLoader.end()
		, std::make_move_iterator(vLoader.begin())
		, std::make_move_iterator(vLoader.end()));

	m_loadWork.reset();
}

void MResourceAsyncLoadSystem::MainThreadLoad(const std::list<std::shared_ptr<MResourceLoader>>& vLoader)
{
	for (auto& pLoader : vLoader)
	{
		if (!pLoader->pResource->Load(std::move(pLoader->pResourceData)))
		{
			GetEngine()->GetLogger()->Error("Load Resource failed: [path: {}]", pLoader->strResourcePath.c_str());
		}
		else
		{
			pLoader->pResource->OnReload();
		}
	}
}