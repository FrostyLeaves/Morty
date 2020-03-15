#include "MResourceManager.h"
#include "MResource.h"
#include "MModelResource.h"
#include "MShaderResource.h"
#include "MMaterialResource.h"
#include "MTextureResource.h"
#include "MResourceLoader.h"

#include <vector>

#define REGISTER_RESOURCE_TYPE(Type, ResourceClass, ...) \
{ \
m_tResourceLoader[Type] = new MResourceLoaderTemp<ResourceClass>(); \
std::vector<MString> vSuffixList = {__VA_ARGS__}; \
for (unsigned int i = 0; i < vSuffixList.size(); ++i) \
m_tResSuffixToType[vSuffixList[i]] = Type; \
} \


MResourceManager::MResourceManager()
	: m_pResourceDB(new MIDPool<MResourceID>())
	, m_pEngine(nullptr)
	, m_bReloadEnabled(false)
{
	REGISTER_RESOURCE_TYPE(MEResourceType::Model, MModelResource, "fbx", "obj", "dae", "blend" );
	REGISTER_RESOURCE_TYPE(MEResourceType::Shader, MShaderResource, SUFFIX_VERTEX_SHADER, SUFFIX_PIXEL_SHADER );
	REGISTER_RESOURCE_TYPE(MEResourceType::Material, MMaterialResource, "mtl");
	REGISTER_RESOURCE_TYPE(MEResourceType::Texture, MTextureResource, "png", "bmp", "tga", "jpg");
}

MResourceManager::~MResourceManager()
{
	for (std::map<MEResourceType, MResourceLoader*>::iterator iter = m_tResourceLoader.begin(); iter != m_tResourceLoader.end(); ++iter)
	{
		delete iter->second;
	}
	m_tResourceLoader.clear();
	for (std::map<MString, MResource*>::iterator iter = m_tPathResources.begin(); iter != m_tPathResources.end(); ++iter)
	{
		delete iter->second;
	}
//	m_tIDResources.clear();
	m_tPathResources.clear();

	delete m_pResourceDB;
}

MResourceManager::MEResourceType MResourceManager::GetResourceType(const MString& strResourcePath)
{
	MString suffix = MResource::GetSuffix(strResourcePath);
	return m_tResSuffixToType[suffix];
}

MResource* MResourceManager::LoadResource(const MString& strResourcePath, const MEResourceType& eType/* = MEResourceType::Default*/)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
		return iter->second;

	MResource* pResource = nullptr;
	MResourceLoader* pLoader = nullptr;
	if (MEResourceType::Default != eType)
		pLoader = m_tResourceLoader[eType];
	else
		pLoader = m_tResourceLoader[GetResourceType(strResourcePath)];

	if (pLoader)
	{
		if(pResource = pLoader->Load(this, strResourcePath))
			m_tPathResources[strResourcePath] = pResource;
	}

	return pResource;
}


void MResourceManager::UnloadResource(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter == m_tPathResources.end())
		return;

// 	std::map<MResourceID, MResource*>::iterator iditer = m_tIDResources.find(iter->second->m_unResourceID);
// 	if (iditer != m_tIDResources.end())
// 		m_tIDResources.erase(iditer);

	delete iter->second;
	m_tPathResources.erase(iter);
}

MResource* MResourceManager::Create(const MEResourceType& eType)
{
	if (MResourceLoader* pLoader = m_tResourceLoader[eType])
	{
		return pLoader->Create(this);
	}

	return nullptr;
}

void MResourceManager::Reload(const MString& strResourcePath)
{
	if (GetReloadEnabled())
	{
		std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
		if (iter != m_tPathResources.end())
		{
			iter->second->Load(strResourcePath);
			iter->second->OnReload(MResource::EResReloadType::EDefault);
		}
	}
}
