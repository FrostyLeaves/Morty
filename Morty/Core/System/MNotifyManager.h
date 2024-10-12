/**
 * @File         MNotifyManager
 * 
 * @Created      2021-08-06 13:58:11
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"
#include "Scene/MManager.h"

namespace morty
{

typedef std::function<void(MComponent* pComponent)> MNotifyFunction;

class MORTY_API                                     MNotifyManager : public IManager
{
    MORTY_CLASS(MNotifyManager)

public:
    MNotifyManager();

    virtual ~MNotifyManager();

public:
    void
    SendNotify(const char* strNotifyName, MScene* pScene, const MComponentID& senderID);

    void RegisterNotify(const char* strNotifyName, MNotifyFunction func);

    void UnregisterNotify(const char* strNotifyName, MNotifyFunction func);

private:
    std::map<const char*, std::vector<MNotifyFunction>> m_notifyTable;
};

}// namespace morty