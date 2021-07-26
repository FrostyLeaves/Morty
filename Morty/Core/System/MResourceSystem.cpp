#include "MResourceSystem.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MResource.h"
#include "MResourceLoader.h"

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
	DELETE_CLEAR_MAP(m_tResourceLoader);
	DELETE_CLEAR_MAP(m_tResources);
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

MResource* MResourceSystem::LoadResource(const MString& strResourcePath, const MType* type/* = nullptr*/)
{
	if (strResourcePath.empty())
		return nullptr;

	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		return iter->second;

	MResource* pResource = nullptr;
	MResourceLoader* pLoader = nullptr;
	if (nullptr == type)
		type = GetResourceType(strResourcePath);

	if (pLoader = m_tResourceLoader[type])
	{
		for (MString strSearchPath : m_vSearchPath)
		{
			std::string strFullpath = strSearchPath + strResourcePath;
			std::ifstream ifs(strFullpath.c_str(), std::ios::binary);
			if (!ifs.good())
				continue;

			ifs.close();
			if (pResource = pLoader->Load(this, strFullpath))
			{
				m_tPathResources[strResourcePath] = pResource;
				break;
			}

			GetEngine()->GetLogger()->Error("Load Resource try to find: [path: %s]", (strSearchPath + "/" + strResourcePath).c_str());
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

void MResourceSystem::UnloadResource(MResource* pResource)
{
	if (nullptr == pResource)
		return;

	if (!pResource->GetResourcePath().empty())
	{
		m_tPathResources.erase(pResource->GetResourcePath());
	}

	m_tResources.erase(pResource->GetResourceID());

	pResource->OnDelete();
	delete pResource;
}

void MResourceSystem::Reload(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
	{
		iter->second->Load(strResourcePath);
		iter->second->OnReload(MResource::EResReloadType::EDefault);
	}
}

MResource* MResourceSystem::FindResourceByID(const MResourceID& unID)
{
	std::map<MResourceID, MResource*>::iterator iter = m_tResources.find(unID);

	if (iter == m_tResources.end())
		return nullptr;

	return iter->second;
}

void MResourceSystem::MoveTo(MResource* pResource, const MString& strTargetPath)
{
	MString strOldPath = pResource->m_strResourcePath;
	m_tPathResources.erase(strOldPath);

	MResource* pTargetResource = m_tPathResources[strTargetPath];
	
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
