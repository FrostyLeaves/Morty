#include "System/MResourceSystem.h"

#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"

#include <vector>
#include <cassert>
#include <fstream>

#define REGISTER_RESOURCE_TYPE(Type, ResourceClass, ...) \
{ \
m_tResourceLoader[Type] = new MResourceLoaderTemp<ResourceClass>(); \
std::vector<MString> vSuffixList = {__VA_ARGS__}; \
for (uint32_t i = 0; i < vSuffixList.size(); ++i) \
m_tResSuffixToType[vSuffixList[i]] = Type; \
} \


MResourceSystem::MResourceSystem()
	: m_ResourceDB()
	, m_vSearchPath({ "" })
{
}

MResourceSystem::~MResourceSystem()
{
	//DELETE_CLEAR_MAP(m_tResourceLoader);
	//DELETE_CLEAR_MAP(m_tResources);
}

void MResourceSystem::SetSearchPath(const std::vector<MString>& vSearchPath)
{
	m_vSearchPath = { "" }; //empty for absolute path.

	for (const MString& strPath : vSearchPath)
	{
		if (!strPath.empty() && strPath.back() != '/')
		{
			m_vSearchPath.push_back(strPath + '/');
		}
		else
		{
			m_vSearchPath.push_back(strPath);
		}
	}
}

const MType* MResourceSystem::GetResourceType(const MString& strResourcePath)
{
	MString suffix = MResource::GetSuffix(strResourcePath);
	return m_tResSuffixToType[suffix];
}

MString MResourceSystem::GetFullPath(const MString& strRelativePath)
{
	for (MString& strSearchPath : m_vSearchPath)
	{
		std::string strFullpath = strSearchPath + strRelativePath;
		std::ifstream ifs(strFullpath.c_str(), std::ios::binary);
		if (ifs.good())
			return strFullpath;
	}

	return "";
}

std::shared_ptr<MResource> MResourceSystem::LoadResource(const MString& strResourcePath, const MType* type/* = nullptr*/)
{
	if (strResourcePath == "Default_White")
	{
		int a = 0;
		++a;
	}

	if (strResourcePath.empty())
		return nullptr;

	std::map<MString, std::shared_ptr<MResource>>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		return iter->second;

	std::shared_ptr<MResource> pResource = nullptr;
	MResourceLoader* pLoader = nullptr;
	if (nullptr == type)
		type = GetResourceType(strResourcePath);

	if (pLoader = m_tResourceLoader[type])
	{
		MString strFullPath = GetFullPath(strResourcePath);

		if (pResource = pLoader->Load(this, strFullPath, strResourcePath))
		{
			m_tPathResources[strResourcePath] = pResource;
		}
		else
		{
			GetEngine()->GetLogger()->Error("Load Resource try to find: [path: %s]", strFullPath.empty() ? strResourcePath.c_str() : strFullPath.c_str());
		}
	}

	if (nullptr == pResource)
	{
#ifdef _DEBUG
	//	assert(false);
#endif
		GetEngine()->GetLogger()->Error("Load Resource failed: [path: %s]", strResourcePath.c_str());
	}

	return pResource;
}

void MResourceSystem::UnloadResource(std::shared_ptr<MResource> pResource)
{
	if (nullptr == pResource)
		return;

	if (!pResource->GetResourcePath().empty())
	{
		m_tPathResources.erase(pResource->GetResourcePath());
	}

	m_tResources.erase(pResource->GetResourceID());

	pResource->OnDelete();
	pResource = nullptr;
}

void MResourceSystem::Reload(const MString& strResourcePath)
{
	std::map<MString, std::shared_ptr<MResource>>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
	{
		iter->second->Load(strResourcePath);
		iter->second->OnReload();
	}
}

std::shared_ptr<MResource> MResourceSystem::FindResourceByID(const MResourceID& unID)
{
	std::map<MResourceID, std::shared_ptr<MResource>>::iterator iter = m_tResources.find(unID);

	if (iter == m_tResources.end())
		return nullptr;

	return iter->second;
}

void MResourceSystem::MoveTo(std::shared_ptr<MResource> pResource, const MString& strTargetPath)
{
	MString strOldPath = pResource->m_strResourcePath;
	m_tPathResources.erase(strOldPath);

	std::shared_ptr<MResource> pTargetResource = m_tPathResources[strTargetPath];
	
	pResource->m_strResourcePath = strTargetPath;
	m_tPathResources[strTargetPath] = pResource;

	if(pTargetResource)
	{
		//Set As Memory Resource
		pTargetResource->m_strResourcePath = "";
		for (MResourceKeeper* pKeeper : pTargetResource->m_vKeeper)
		{
			pKeeper->SetResource(pResource);
		}
	}
}


void MResourceSystem::Release()
{
	for (auto&& pr : m_tResources)
	{
		pr.second->OnDelete();
		pr.second = nullptr;
	}

	m_tResources.clear();
	m_tPathResources.clear();
}
