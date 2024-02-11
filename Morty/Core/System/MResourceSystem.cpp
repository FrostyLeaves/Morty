#include "System/MResourceSystem.h"

#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"

#include <vector>
#include <cassert>
#include <fstream>

#include "Resource/MResourceAsyncLoadSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MResourceSystem, MISystem)

MResourceSystem::MResourceSystem()
	: m_ResourceDB()
	, m_vSearchPath({ "" })
{
}

MResourceSystem::~MResourceSystem()
{
}

std::shared_ptr<MResource> MResourceSystem::CreateResource(const MType* type)
{
	if (!MTypeClass::IsType(type, MResource::GetClassType()))
	{
		return nullptr;
	}

	std::shared_ptr<MResource> pResource = std::shared_ptr<MResource>(static_cast<MResource*>(MTypeClass::New(type)));
	if (!pResource)
	{
		MORTY_ASSERT(pResource);
		return nullptr;
	}

	pResource->m_self = pResource;
	pResource->m_unResourceID = m_ResourceDB.GetNewID();
	pResource->m_pEngine = GetEngine();
	m_tResources[pResource->m_unResourceID] = pResource;

	pResource->OnCreated();
	return pResource;
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

std::shared_ptr<MResource> MResourceSystem::LoadResource(const MString& strResourcePath, bool bAsyncLoad)
{
	if (strResourcePath.empty())
	{
		return nullptr;
	}

	const auto iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
	{
		return iter->second;
	}

	const MString strFullPath = GetFullPath(strResourcePath);
	if (strFullPath.empty())
	{
		return nullptr;
	}

	auto pLoader = CreateLoader(strResourcePath);
	if (!pLoader)
	{
		return nullptr;
	}

	std::shared_ptr<MResource> pResource = CreateResource(pLoader->ResourceType());
	if (!pResource)
	{
		GetEngine()->GetLogger()->Error("Create Resource failed: [path: {}]", strResourcePath.c_str());
		return nullptr;
	}

	pResource->m_strResourcePath = strResourcePath;
	m_tPathResources[strResourcePath] = pResource;


	if (bAsyncLoad)
	{
		pLoader->strResourcePath = strResourcePath;
		pLoader->strResourceFullPath = strFullPath;
		pLoader->pResource = pResource;
		GetEngine()->FindSystem<MResourceAsyncLoadSystem>()->AddLoader(pLoader);
	}
	else
	{
		if (!pResource->Load(pLoader->LoadResource(strFullPath)))
		{
			GetEngine()->GetLogger()->Error("Load Resource failed: [path: {}]", pLoader->strResourcePath.c_str());
		}
	}

	return pResource;
}

std::unique_ptr<MResourceData> MResourceSystem::LoadResourceData(const MString& strResourcePath)
{
	const MString strFullPath = GetFullPath(strResourcePath);
	if (strFullPath.empty())
	{
		return nullptr;
	}

	auto pLoader = CreateLoader(strResourcePath);
	if (!pLoader)
	{
		return nullptr;
	}

	return pLoader->LoadResource(strFullPath);
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

void MResourceSystem::SaveResource(std::shared_ptr<MResource> pResource)
{
    if(!pResource)
    {
		return;
    }

	std::unique_ptr<MResourceData> pResourceData = nullptr;
	pResource->SaveTo(pResourceData);

	if (!pResourceData)
	{
		return;
	}

	MString strResourcePath = pResource->GetResourcePath();

	if (strResourcePath.empty())
	{
		MORTY_ASSERT(strResourcePath.empty());
		return;
	}

	SaveResource(pResourceData, strResourcePath);
}

void MResourceSystem::SaveResource(const std::unique_ptr<MResourceData>& pResourceData, const MString& strOutputPath)
{
	std::vector<MByte> data = pResourceData->SaveBuffer();
	MFileHelper::WriteData(strOutputPath, data);
}

std::shared_ptr<MResourceLoader> MResourceSystem::CreateLoader(const MString& strResourcePath)
{
	const MString suffix = MResource::GetSuffix(strResourcePath);
	const auto findResult = m_tResourceLoader.find(suffix);
	
	if (findResult != m_tResourceLoader.end())
	{
		return findResult->second();
	}

	return nullptr;
}

void MResourceSystem::Reload(const MString& strResourcePath)
{
	const std::map<MString, std::shared_ptr<MResource>>::iterator iter = m_tPathResources.find(strResourcePath);
	if (iter != m_tPathResources.end())
	{
		const MString strFullPath = GetFullPath(strResourcePath);

		if (auto pLoader = CreateLoader(strResourcePath))
		{
			if (auto pResourceData = pLoader->LoadResource(strFullPath))
			{
				iter->second->Load(std::move(pResourceData));
				iter->second->OnReload();
			}
		}
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
		for (MResourceRef* pKeeper : pTargetResource->m_vKeeper)
		{
			pKeeper->SetResource(pResource);
		}
	}
}

void MResourceSystem::SaveTo(std::shared_ptr<MResource> pResource, const MString& strTargetPath)
{
	std::unique_ptr<MResourceData> pResourceData = nullptr;
	pResource->SaveTo(pResourceData);
	auto buffer = pResourceData->SaveBuffer();
	MFileHelper::WriteData(strTargetPath, buffer);
}

void MResourceSystem::Release()
{
	for (auto pr : m_tResources)
	{
		pr.second->OnDelete();
		pr.second = nullptr;
	}

	m_tResources.clear();
	m_tPathResources.clear();
}
