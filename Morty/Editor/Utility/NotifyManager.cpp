#include "Utility/NotifyManager.h"

using namespace morty;

NotifyManager::NotifyManager()
{

}

NotifyManager::~NotifyManager()
{

}

NotifyManager* NotifyManager::GetInstance()
{
	static NotifyManager mgr;
	return &mgr;
}

void NotifyManager::SendNotify(const MString& strNotifyName, const MVariant& param /*= MVariant()*/)
{
	std::map<MString, NotifyGroup*>::iterator iter = m_tNotifyTable.find(strNotifyName);
	if (iter != m_tNotifyTable.end())
	{
		for (NotifyFunction& func : iter->second->m_vFunction)
		{
			func(param);
		}
	}
}

void NotifyManager::RegisterNotify(const MString& strNotifyName, void* pRegister, const NotifyFunction& func)
{
	std::map<MString, NotifyGroup*>::iterator iter = m_tNotifyTable.find(strNotifyName);

	NotifyGroup* pGroup = nullptr;

	if (iter == m_tNotifyTable.end())
	{
		pGroup = new NotifyGroup();
		m_tNotifyTable[strNotifyName] = pGroup;
	}
	else
		pGroup = iter->second;

	for (unsigned int i = 0; i < pGroup->m_vRegister.size(); ++i)
	{
		if (pGroup->m_vRegister[i] == pRegister)
		{
			pGroup->m_vFunction[i] = func;
			return;
		}
	}

	pGroup->m_vRegister.push_back(pRegister);
	pGroup->m_vFunction.push_back(func);

}

void NotifyManager::UnRegisterAll()
{
	for (std::pair<const MString, NotifyGroup*>& pair : m_tNotifyTable)
	{
		delete pair.second;
	}

	m_tNotifyTable.clear();
}
