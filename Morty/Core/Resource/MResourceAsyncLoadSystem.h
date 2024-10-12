/**
 * @File         MResourceAsyncLoadSystem
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Thread/MThreadWork.h"

namespace morty
{

class MTaskNode;
class MResourceLoader;
class MResourceSystem;
class MORTY_API MResourceAsyncLoadSystem : public MISystem
{
public:
    void Initialize() override;

    void AddLoader(std::shared_ptr<MResourceLoader>& pLoader);

    void EngineTick(const float& fDelta) override;

    void AnyThreadLoad(const std::list<std::shared_ptr<MResourceLoader>>& vLoader);

    void MainThreadLoad(const std::list<std::shared_ptr<MResourceLoader>>& vLoader);

private:
    std::list<std::shared_ptr<MResourceLoader>> m_pendingLoader;
    std::list<std::shared_ptr<MResourceLoader>> m_finishedLoader;

    std::optional<MThreadWork>                  m_loadWork;
};

}// namespace morty