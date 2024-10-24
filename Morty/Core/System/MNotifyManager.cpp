#include "System/MNotifyManager.h"

#include "Scene/MScene.h"
#include "Utility/MFunction.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MNotifyManager, IManager)

MNotifyManager::MNotifyManager() {}

MNotifyManager::~MNotifyManager() {}

void MNotifyManager::SendNotify(
        const char*         strNotifyName,
        MScene*             pScene,
        const MComponentID& senderID
)
{
    auto findResult = m_notifyTable.find(strNotifyName);
    if (findResult != m_notifyTable.end())
    {
        for (MNotifyFunction& func: findResult->second)
        {
            MComponent* pSender = pScene->GetComponent(senderID);
            func(pSender);
        }
    }
}

void MNotifyManager::RegisterNotify(const char* strNotifyName, MNotifyFunction func)
{
    UNION_PUSH_BACK_VECTOR<MNotifyFunction>(
            m_notifyTable[strNotifyName],
            func,
            [](const MNotifyFunction& a, const MNotifyFunction& b) {
                if (a.target_type() != b.target_type()) return false;

                if (a.target<MNotifyFunction>() != b.target<MNotifyFunction>())
                {
                    return false;
                }
                //if (*a.target<void(MComponent*)>() != *b.target<void(MComponent*)>())
                //	return false;

                return true;
            }
    );
}

void MNotifyManager::UnregisterNotify(const char* strNotifyName, MNotifyFunction func)
{
    auto findResult = m_notifyTable.find(strNotifyName);
    if (findResult != m_notifyTable.end())
    {
        ERASE_FIRST_VECTOR<MNotifyFunction>(
                findResult->second,
                func,
                [](const MNotifyFunction& a, const MNotifyFunction& b) {
                    if (a.target_type() != b.target_type()) return false;

                    if (*a.target<void (*)(MComponent*)>() !=
                        *b.target<void (*)(MComponent*)>())
                        return false;

                    return false;
                }
        );

        if (findResult->second.empty()) { m_notifyTable.erase(findResult); }
    }
}
