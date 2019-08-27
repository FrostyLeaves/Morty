#include "MResourceManager.h"
#include "MResource.h"
#include "MModelResource.h"
#include "MShaderResource.h"
#include "MResourceLoader.h"

#include <vector>

#define REGISTER_RESOURCE_TYPE(Type, ResourceClass, ...) \
{ \
m_tResourceLoader[Type] = new MResourceLoaderTemp<ResourceClass>(); \
std::vector<MString> vSuffixList = {__VA_ARGS__}; \
for (int i = 0; i < vSuffixList.size(); ++i) \
m_tResSuffixToType[vSuffixList[i]] = Type; \
} \


MResourceManager::MResourceManager()
	: m_pResourceDB(new MIDPool<MResourceID>())
{
	REGISTER_RESOURCE_TYPE(MEResourceType::Model, MModelResource, "fbx", "obj" );
	REGISTER_RESOURCE_TYPE(MEResourceType::Shader, MShaderResource, SUFFIX_VERTEX_SHADER, SUFFIX_PIXEL_SHADER );
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
	MString suffix = MResource::GetSuffix(strResourcePath);
	return m_tResSuffixToType[suffix];
}

MResource* MResourceManager::Load(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		return iter->second;

	MResource* pResource = nullptr;

	if (MResourceLoader* pLoader = m_tResourceLoader[GetResourceType(strResourcePath)])
	{
		pResource = pLoader->Load(this, strResourcePath);
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
