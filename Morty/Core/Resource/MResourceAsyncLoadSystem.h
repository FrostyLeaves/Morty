/**
 * @File         MResourceAsyncLoadSystem
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRESOURCE_ASYNC_LOAD_SYSTEM_H_
#define _M_MRESOURCE_ASYNC_LOAD_SYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Thread/MThreadWork.h"

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

    std::list<std::shared_ptr<MResourceLoader>> m_vPendingLoader;
    std::list<std::shared_ptr<MResourceLoader>> m_vFinishedLoader;

    std::optional<MThreadWork> m_loadWork;
};


#endif
