#include "Utility/NotifyManager.h"

using namespace morty;

NotifyManager::NotifyManager() {}

NotifyManager::~NotifyManager() {}

NotifyManager* NotifyManager::GetInstance()
{
    static NotifyManager mgr;
    return &mgr;
}

void NotifyManager::
        SendNotify(const MString& strNotifyName, const MVariant& param /*= MVariant()*/)
{
    std::map<MString, NotifyGroup*>::iterator iter = m_notifyTable.find(strNotifyName);
    if (iter != m_notifyTable.end())
    {
        for (NotifyFunction& func: iter->second->m_function) { func(param); }
    }
}

void NotifyManager::RegisterNotify(
        const MString&        strNotifyName,
        void*                 pRegister,
        const NotifyFunction& func
)
{
    std::map<MString, NotifyGroup*>::iterator iter = m_notifyTable.find(strNotifyName);

    NotifyGroup*                              pGroup = nullptr;

    if (iter == m_notifyTable.end())
    {
        pGroup                       = new NotifyGroup();
        m_notifyTable[strNotifyName] = pGroup;
    }
    else
        pGroup = iter->second;

    for (unsigned int i = 0; i < pGroup->m_register.size(); ++i)
    {
        if (pGroup->m_register[i] == pRegister)
        {
            pGroup->m_function[i] = func;
            return;
        }
    }

    pGroup->m_register.push_back(pRegister);
    pGroup->m_function.push_back(func);
}

void NotifyManager::UnRegisterAll()
{
    for (std::pair<const MString, NotifyGroup*>& pair: m_notifyTable)
    {
        delete pair.second;
    }

    m_notifyTable.clear();
}
