/**
 * @File         MResourceSystem
 * 
 * @Created      2019-08-11 13:40:09
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "Utility/MIDPool.h"
#include "Engine/MSystem.h"
#include "Engine/MEngine.h"
#include "Resource/MResourceLoader.h"

#include <vector>
#include <map>

class MResource;
class MResourceLoader;

class MORTY_API MResourceSystem : public MISystem
{
public:
	MORTY_CLASS(MResourceSystem)

	MResourceSystem();
	virtual ~MResourceSystem();

	template <typename TYPE>
	bool RegisterResourceLoader();

	template<typename TYPE>
	std::shared_ptr<TYPE> CreateResource();

    std::shared_ptr<MResource> CreateResource(const MType* type);

	template<typename TYPE>
	std::shared_ptr<TYPE> CreateResource(const MString& strResourcePath);

	void SetSearchPath(const std::vector<MString>& vSearchPath);
	std::vector<MString> GetSearchPath() const { return m_vSearchPath; }

	MString GetFullPath(const MString& strRelativePath);

	std::shared_ptr<MResource> LoadResource(const MString& strResourcePath, bool bAsyncLoad = false);
	std::unique_ptr<MResourceData> LoadResourceData(const MString& strResourcePath);
    void UnloadResource(std::shared_ptr<MResource> pResource);
	void SaveResource(std::shared_ptr<MResource> pResource);
	void SaveResource(const std::unique_ptr<MResourceData>& pResourceData, const MString& strOutputPath);

	void Reload(const MString& strResourcePath);
	std::shared_ptr<MResourceLoader> CreateLoader(const MString& strResourcePath);

	std::shared_ptr<MResource> FindResourceByID(const MResourceID& unID);

	std::map<MResourceID, std::shared_ptr<MResource>>* GetAllResources() { return &m_tResources; }

	void MoveTo(std::shared_ptr<MResource> pResource, const MString& strTargetPath);

	void SaveTo(std::shared_ptr<MResource> pResource, const MString& strTargetPath);

	virtual void Release() override;

private:
	
	std::map<MResourceID, std::shared_ptr<MResource>> m_tResources;
	std::map<MString, std::shared_ptr<MResource>> m_tPathResources;

	MIDPool<MResourceID> m_ResourceDB;
	std::map<MString, std::function<std::shared_ptr<MResourceLoader>()>> m_tResourceLoader;

	std::vector<MString> m_vSearchPath;

};

template <typename TYPE>
bool MResourceSystem::RegisterResourceLoader()
{
	MString strTypeName = TYPE::GetResourceTypeName();
	std::vector<MString> vSuffixNameList = TYPE::GetSuffixList();
	
	for (const MString& suffix : vSuffixNameList)
	{
		if (m_tResourceLoader.find(suffix) != m_tResourceLoader.end())
		{
			GetEngine()->GetLogger()->Error("file suffix is already registed. suffix: {}", suffix.c_str());
			continue;
		}
		m_tResourceLoader[suffix] = []() { return std::make_unique<TYPE>(); };
	}


	return true;
}

template<typename TYPE>
std::shared_ptr<TYPE> MResourceSystem::CreateResource()
{
	return std::static_pointer_cast<TYPE>(CreateResource(TYPE::GetClassType()));
}

template<typename TYPE>
std::shared_ptr<TYPE> MResourceSystem::CreateResource(const MString& strResourcePath)
{
	if (std::shared_ptr<MResource> pResource = m_tPathResources[strResourcePath])
		return std::dynamic_pointer_cast<TYPE>(pResource);

	std::shared_ptr<TYPE> pResource = CreateResource<TYPE>();
	pResource->m_strResourcePath = strResourcePath;
	m_tPathResources[strResourcePath] = pResource;

	return pResource;
}
