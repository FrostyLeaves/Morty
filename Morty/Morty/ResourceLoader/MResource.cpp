#include "MResource.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MFileHelper.h"

MResource::MResource()
: m_unResourceID(0)
, m_unResourceType(0)
, m_pEngine(nullptr)
{
    
}

MResource::~MResource()
{
	for (MResourceKeeper* pHolder : m_vHolder)
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
	GetResourceManager()->UnloadResource(this);
} 

void MResource::OnReload(const unsigned int& eReloadType)
{
	for (MResourceKeeper* pHolder : m_vHolder)
	{
		if (pHolder->m_funcReloadCallback)
			pHolder->m_funcReloadCallback(eReloadType);
	}
}

MResourceKeeper::MResourceKeeper()
	: m_funcReloadCallback(nullptr)
	, m_pResource(nullptr)
{

}

MResourceKeeper::MResourceKeeper(MResource* pResource)
	: m_funcReloadCallback(nullptr)
	, m_pResource(nullptr)
{
	SetResource(pResource);
}

MResourceKeeper::MResourceKeeper(const MResourceKeeper& cHolder)
	: m_funcReloadCallback(cHolder.m_funcReloadCallback)
	, m_pResource(nullptr)
{
	SetResource(cHolder.m_pResource);
}

MResourceKeeper::~MResourceKeeper()
{
	SetResource(nullptr);
}

void MResourceKeeper::SetResource(MResource* pResource)
{
	if (m_pResource)
	{
		std::vector<MResourceKeeper*>::iterator iter = std::find(m_pResource->m_vHolder.begin(), m_pResource->m_vHolder.end(), this);
		if (m_pResource->m_vHolder.end() != iter)
		{
			m_pResource->m_vHolder.erase(iter);
		}

		m_pResource->SubRef();
	}
	
	if (m_pResource = pResource)
	{
		m_pResource->AddRef();
		m_pResource->m_vHolder.push_back(this);
	}
}

const MResourceKeeper& MResourceKeeper::operator=(const MResourceKeeper& keeper)
{
	m_funcReloadCallback = keeper.m_funcReloadCallback;
	SetResource(keeper.m_pResource);

	return keeper;
}
