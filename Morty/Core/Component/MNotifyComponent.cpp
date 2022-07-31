#include "Component/MNotifyComponent.h"


void MComponentNotifyInfo::AddNotifyFunction(const MType* pComponentType, const std::function<void()>& callback)
{
	for (auto& cf : m_vComponentNotifyCallFunction)
	{
		if (cf.pComponentType == pComponentType)
		{
			cf.function = callback;
			return;
		}
	}

	MComponentNotifyCallFunction cf;
	cf.pComponentType = pComponentType;
	cf.function = callback;
	m_vComponentNotifyCallFunction.push_back(cf);
}

void MComponentNotifyInfo::RemoveNotifyFunction(const MType* pComponentType)
{
	for (auto iter = m_vComponentNotifyCallFunction.begin(); iter != m_vComponentNotifyCallFunction.end(); ++iter)
	{
		if (iter->pComponentType == pComponentType)
		{
			m_vComponentNotifyCallFunction.erase(iter);
			return;
		}
	}
}

MNotifyComponent::MNotifyComponent()
	: MComponent()
{

}

MNotifyComponent::~MNotifyComponent()
{

}

void MNotifyComponent::SendNotify(const MString& strNotify)
{
	auto findResult = m_tComponentNotify.find(strNotify);

	if (findResult == m_tComponentNotify.end())
		return;

	MComponentNotifyInfo* pNotifyInfo = findResult->second;
	if (!pNotifyInfo)
		return;

	for (auto& callfunction : pNotifyInfo->m_vComponentNotifyCallFunction)
	{
		callfunction.function();
	}
}

void MNotifyComponent::RegisterComponentNotify(const MString& strNotify, const MType* pComponentType, const std::function<void()>& callback)
{
	MComponentNotifyInfo* pNotifyInfo = nullptr;

	auto findResult = m_tComponentNotify.find(strNotify);

	if (findResult == m_tComponentNotify.end())
	{
		pNotifyInfo = new MComponentNotifyInfo();
		m_tComponentNotify.insert(std::make_pair(strNotify, pNotifyInfo));
	}
	else
	{
		pNotifyInfo = findResult->second;
	}

	if (!pNotifyInfo)
		return;

	pNotifyInfo->AddNotifyFunction(pComponentType, callback);
}

void MNotifyComponent::UnregisterComponentNotify(const MString& strNotifyName, const MType* pComponentType)
{
	auto findResult = m_tComponentNotify.find(strNotifyName);

	if (findResult == m_tComponentNotify.end())
		return;

	MComponentNotifyInfo* pNotifyInfo = findResult->second;
	m_tComponentNotify.erase(findResult);

	if (pNotifyInfo)
	{
		delete pNotifyInfo;
		pNotifyInfo = nullptr;
	}
}
