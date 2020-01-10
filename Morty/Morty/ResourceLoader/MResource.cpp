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
	size_t index = strPath.find_last_of('.');
	if (index >= strPath.size())
		return "";

	MString suffix = strPath.substr(index + 1, strPath.size());
	for (char& c : suffix)
	{
		if ('A' <= c && c <= 'Z')
			c += ('a' - 'A');
	}

	return suffix;
}

MString MResource::GetFolder(const MString& strPath)
{
	MString strRegularPath = strPath;
	for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
	{
		if (*iter == '\\' || *iter == '/')
		{
			return MString(strRegularPath.begin(), iter.base() - 1);
		}
	}

	return MString();
}

MString MResource::GetFileName(const MString& strPath)
{
	MString strRegularPath = strPath;
	for (MString::reverse_iterator iter = strRegularPath.rbegin(); iter != strRegularPath.rend(); ++iter)
	{
		if (*iter == '\\' || *iter == '/')
		{
			return MString(iter.base(), strRegularPath.end());
		}
	}

	return strPath;
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
