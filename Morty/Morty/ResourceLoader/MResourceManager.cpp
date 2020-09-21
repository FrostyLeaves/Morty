#include "MResourceManager.h"
#include "MResource.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"
#include "Shader/MShaderResource.h"
#include "Model/MSkeletonResource.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"
#include "Model/MSkeletalAnimationResource.h"
#include "Node/MNodeResource.h"
#include "MResourceLoader.h"

#include "MFunction.h"

#include <vector>

#define REGISTER_RESOURCE_TYPE(Type, ResourceClass, ...) \
{ \
m_tResourceLoader[Type] = new MResourceLoaderTemp<ResourceClass>(); \
std::vector<MString> vSuffixList = {__VA_ARGS__}; \
for (uint32_t i = 0; i < vSuffixList.size(); ++i) \
m_tResSuffixToType[vSuffixList[i]] = Type; \
} \


MResourceManager::MResourceManager()
	: m_ResourceDB()
	, m_pEngine(nullptr)
	, m_vSearchPath({ "." })
{
	REGISTER_RESOURCE_TYPE(MEResourceType::SkelAnim, MSkeletalAnimationResource, SUFFIX_SKELANIM);
	REGISTER_RESOURCE_TYPE(MEResourceType::Mesh, MMeshResource, SUFFIX_MESH);
	REGISTER_RESOURCE_TYPE(MEResourceType::Model, MModelResource, SUFFIX_MODEL);
	REGISTER_RESOURCE_TYPE(MEResourceType::Skeleton, MSkeletonResource, SUFFIX_SKELETON);
	REGISTER_RESOURCE_TYPE(MEResourceType::Shader, MShaderResource, SUFFIX_VERTEX_SHADER, SUFFIX_PIXEL_SHADER );
	REGISTER_RESOURCE_TYPE(MEResourceType::Material, MMaterialResource, SUFFIX_MATERIAL);
	REGISTER_RESOURCE_TYPE(MEResourceType::Texture, MTextureResource, "png", "bmp", "tga", "jpg");
	REGISTER_RESOURCE_TYPE(MEResourceType::Node, MNodeResource, "node");
}

MResourceManager::~MResourceManager()
{
	DELETE_CLEAR_MAP(m_tResourceLoader);
	DELETE_CLEAR_MAP(m_tResources);
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
	MEResourceType eResourceType = eType;
	if (MEResourceType::Default == eResourceType)
		eResourceType = GetResourceType(strResourcePath);

	if (pLoader = m_tResourceLoader[eResourceType])
	{
		if (pResource = pLoader->Load(this, strResourcePath))
		{
			m_tPathResources[strResourcePath] = pResource;
		}
		else
		{
			for (MString strSearchPath : m_vSearchPath)
			{
				if (pResource = pLoader->Load(this, strSearchPath + "/" + strResourcePath))
				{
					m_tPathResources[strResourcePath] = pResource;
					break;
				}

				MLogManager::GetInstance()->Error("Load Resource try to find: [path: %s]", (strSearchPath + "/" + strResourcePath).c_str());
			}
		}
	}

	if (nullptr == pResource)
	{
		MLogManager::GetInstance()->Error("Load Resource not found: [path: %s]", strResourcePath.c_str());
	}

	return pResource;
}

void MResourceManager::UnloadResource(MResource* pResource)
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

void MResourceManager::Reload(const MString& strResourcePath)
{
	std::map<MString, MResource*>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
	{
		iter->second->Load(strResourcePath);
		iter->second->OnReload(MResource::EResReloadType::EDefault);
	}
}

MResource* MResourceManager::FindResourceByID(const MResourceID& unID)
{
	std::map<MResourceID, MResource*>::iterator iter = m_tResources.find(unID);

	if (iter == m_tResources.end())
		return nullptr;

	return iter->second;
}

void MResourceManager::MoveTo(MResource* pResource, const MString& strTargetPath)
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
