#include "Resource/MResource.h"
#include "Engine/MEngine.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

MORTY_INTERFACE_IMPLEMENT(MResource, MTypeClass)

MResource::MResource()
: m_unResourceID(0)
, m_pEngine(nullptr)
{
    
}

MResource::~MResource()
{
	for (MResourceKeeper* pKeeper : m_vKeeper)
	{
		pKeeper->m_pResource = nullptr;
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

MResourceSystem* MResource::GetResourceSystem()
{
	if (nullptr == m_pEngine)
		return nullptr;

	if (MISystem* pSystem = m_pEngine->FindSystem(MResourceSystem::GetClassType()))
	{
		return pSystem->DynamicCast<MResourceSystem>();
	}

	return nullptr;
}

std::shared_ptr<MResource> MResource::GetShared()
{
	return m_self.lock();
}

void MResource::ReplaceFrom(std::shared_ptr<MResource> pResource)
{
	if (pResource->GetType() != GetType())
		return;

	std::vector<MResourceKeeper*> keeps = m_vKeeper;
	m_vKeeper.clear();

	for (MResourceKeeper* pKeeper : keeps)
	{
		//pKeeper->SetResource(pResource);

		pKeeper->m_pResource = pResource;
		pResource->m_vKeeper.push_back(pKeeper);

		if (pKeeper->m_funcReloadCallback)
		{
			pKeeper->m_funcReloadCallback(EResReloadType::EDefault);
		}
	}
}

void MResource::OnReload(const uint32_t& eReloadType)
{
	for (MResourceKeeper* pKeeper : m_vKeeper)
	{
		if (pKeeper->m_funcReloadCallback)
			pKeeper->m_funcReloadCallback(eReloadType);
	}
}

MResourceKeeper::MResourceKeeper()
	: m_funcReloadCallback(nullptr)
	, m_pResource(nullptr)
{

}

MResourceKeeper::MResourceKeeper(std::shared_ptr<MResource> pResource)
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

void MResourceKeeper::SetResource(std::shared_ptr<MResource> pResource)
{
	std::shared_ptr<MResource> pOldResource = m_pResource;
	if (m_pResource)
	{
		std::vector<MResourceKeeper*>::iterator iter = std::find(m_pResource->m_vKeeper.begin(), m_pResource->m_vKeeper.end(), this);
		if (m_pResource->m_vKeeper.end() != iter)
		{
			m_pResource->m_vKeeper.erase(iter);
		}
	}
	
	if (m_pResource = pResource)
	{
		m_pResource->m_vKeeper.push_back(this);
	}
}

const MResourceKeeper& MResourceKeeper::operator=(const MResourceKeeper& keeper)
{
	m_funcReloadCallback = keeper.m_funcReloadCallback;
	SetResource(keeper.m_pResource);

	return keeper;
}

std::shared_ptr<MResource> MResourceKeeper::operator=(std::shared_ptr<MResource> pResource)
{
	m_funcReloadCallback = nullptr;
	SetResource(pResource);

	return pResource;
}
