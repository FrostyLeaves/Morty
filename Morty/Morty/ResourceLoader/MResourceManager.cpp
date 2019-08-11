#include "MResourceManager.h"
#include "MModelLoader.h"
#include "MResource.h"


#define REGISTER_RESOURCE_TYPE(Type, LoaderClass) \
m_tResourceLoader[Type] = new LoaderClass();

MResourceManager::MResourceManager()
	: m_pResourceDB(new MIDPool<MResourceID>())
{
	REGISTER_RESOURCE_TYPE(MEResourceType::Model, MModelLoader);
}

MResourceManager::~MResourceManager()
{
	for (std::map<MEResourceType, MResourceLoader*>::iterator iter = m_tResourceLoader.begin(); iter != m_tResourceLoader.end(); ++iter)
	{
		delete iter->second;
	}

	m_tResourceLoader.clear();
}

MResourceManager::MEResourceType MResourceManager::GetResourceType(const MString& strResourcePath)
{
	MString suffix = strResourcePath.substr(strResourcePath.find_last_of('.') + 1, strResourcePath.size());

	if (suffix == "fbx")
	{
		return MEResourceType::Model;
	}
}

MResource* MResourceManager::Load(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		return iter->second;

	MResource* pResource = nullptr;

	if (MResourceLoader* pLoader = m_tResourceLoader[GetResourceType(strResourcePath)])
	{
		pResource = pLoader->Load(strResourcePath);
		m_tPathResources[strResourcePath] = pResource;
	}

	return pResource;
}

void MResourceManager::Reload(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		iter->second->Load(strResourcePath);
}
