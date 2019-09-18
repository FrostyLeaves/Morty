#include "MNotifyManager.h"
#include "MLogManager.h"


bool MNotifyManager::RegisterNotify(const MString& strNotifyName, MObject* pObject, MNotifyEventFunction function)
{
	std::map<MObject*, std::vector<NotifyRegister>* >::iterator iter = m_tNotifyMap.find(pObject);
	if (iter == m_tNotifyMap.end())
	{
		m_tNotifyMap[pObject] = new std::vector<NotifyRegister>();
	}

	std::vector<NotifyRegister>* & pVector = m_tNotifyMap[pObject];

	for (NotifyRegister& reg : *pVector)
	{
		if (reg.strNotifyName == strNotifyName)
		{
			MLogManager::GetInstance()->Warning("Repeated notifications! NotifyName: \"%s\" has been registered by this object.");
			reg.function = function;
			return true;
		}
	}

	pVector->push_back(NotifyRegister(strNotifyName, function));
	return true;

}

void MNotifyManager::UnRegisterNotify(const MString& strNotifyName, MObject* pObject)
{
	std::map<MObject*, std::vector<NotifyRegister>* >::iterator iter = m_tNotifyMap.find(pObject);
	if (iter == m_tNotifyMap.end())
		return;

	std::vector<NotifyRegister>& vector = *iter->second;

	for (std::vector<NotifyRegister>::iterator it = vector.begin(); it != vector.end(); ++it)
	{
		if (it->strNotifyName == strNotifyName)
		{
			vector.erase(it);
			break;
		}
	}

	if (vector.empty())
	{
		delete iter->second;
		m_tNotifyMap.erase(iter);
	}
}

void MNotifyManager::UnRegisterAllNotifyByObject(MObject* pObject)
{
	std::map<MObject*, std::vector<NotifyRegister>* >::iterator iter = m_tNotifyMap.find(pObject);

	if (iter != m_tNotifyMap.end())
	{
		delete iter->second;
		m_tNotifyMap.erase(iter);
	}
}
