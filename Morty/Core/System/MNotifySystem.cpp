#include "MNotifySystem.h"

#include "MScene.h"
#include "MFunction.h"

MORTY_CLASS_IMPLEMENT(MNotifySystem, MISystem)

MNotifySystem::MNotifySystem()
{

}

MNotifySystem::~MNotifySystem()
{

}

void MNotifySystem::SendNotify(const MString& strNotifyName, MScene* pScene, const MComponentID& senderID)
{
	auto findResult = m_tNotifyTable.find(strNotifyName);
	if (findResult != m_tNotifyTable.end())
	{
		for (MNotifyFunction& func : findResult->second)
		{
			MComponent* pSender = pScene->GetComponent(senderID);
			func(pSender);
		}
	}
}

void MNotifySystem::RegisterNotify(const MString& strNotifyName, MNotifyFunction func)
{
	UNION_PUSH_BACK_VECTOR<MNotifyFunction>(m_tNotifyTable[strNotifyName], func, [](const MNotifyFunction& a, const MNotifyFunction& b) {
		if (a.target_type() != b.target_type())
			return false;

		if (*a.target<void(*)(MComponent*)>() != *b.target<void(*)(MComponent*)>())
			return false;

		return false;
		});
}

void MNotifySystem::UnregisterNotify(const MString& strNotifyName, MNotifyFunction func)
{
	auto findResult = m_tNotifyTable.find(strNotifyName);
	if (findResult != m_tNotifyTable.end())
	{
		ERASE_FIRST_VECTOR<MNotifyFunction>(findResult->second, func, [](const MNotifyFunction& a, const MNotifyFunction& b) {
			if (a.target_type() != b.target_type())
				return false;

			if (*a.target<void(*)(MComponent*)>() != *b.target<void(*)(MComponent*)>())
				return false;

			return false;
			});

		if (findResult->second.empty())
		{
			m_tNotifyTable.erase(findResult);
		}
	}
}
