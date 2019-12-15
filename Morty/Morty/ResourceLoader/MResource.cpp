#include "MResource.h"
#include "MEngine.h"
#include "MResourceManager.h"

MResource::MResource()
: m_unResourceID(0)
, m_pEngine(nullptr)
{
    
}

MResource::~MResource()
{
	for (MResourceHolder* pHolder : m_vHolder)
	{
		pHolder->m_pResource = nullptr;
	}
}

MString MResource::GetSuffix(const MString& strPath)
{
	MString suffix = strPath.substr(strPath.find_last_of('.') + 1, strPath.size());
	for (char& c : suffix)
	{
		if ('A' <= c && c <= 'Z')
			c += ('a' - 'A');
	}

	return suffix;
}

MResourceManager* MResource::GetResourceManager()
{
	return m_pEngine->GetResourceManager();
}

void MResource::OnReferenceZero()
{
	if (!m_strResourcePath.empty())
	{
		GetResourceManager()->UnloadResource(m_strResourcePath);
	}
}

void MResource::OnReload()
{
	for (MResourceHolder* pHolder : m_vHolder)
	{
		if (pHolder->m_funcReloadCallback)
			pHolder->m_funcReloadCallback();
	}
}

MResourceHolder::MResourceHolder(MResource* pResource)
	: m_funcReloadCallback(nullptr)
	, m_pResource(pResource)
{
	if (m_pResource && m_pResource->GetResourceManager()->GetReloadEnabled())
	{
		m_pResource->AddRef();
		m_pResource->m_vHolder.push_back(this);
	}
}

MResourceHolder::~MResourceHolder()
{
	if (m_pResource && m_pResource->GetResourceManager()->GetReloadEnabled())
	{
		std::vector<MResourceHolder*>::iterator iter = std::find(m_pResource->m_vHolder.begin(), m_pResource->m_vHolder.end(), this);
		if (m_pResource->m_vHolder.end() != iter)
		{
			m_pResource->m_vHolder.erase(iter);
		}

		m_pResource->SubRef();

	}
}
