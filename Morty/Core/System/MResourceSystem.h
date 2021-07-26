/**
 * @File         MResourceSystem
 * 
 * @Created      2019-08-11 13:40:09
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRESOURCE_SYSTEM_H_
#define _M_MRESOURCE_SYSTEM_H_
#include "MGlobal.h"

#include "MIDPool.h"
#include "MSystem.h"
#include "MResourceLoader.h"

#include <vector>
#include <map>

class MEngine;
class MIRenderer;
class MResource;
class MResourceLoader;

class MORTY_API MResourceSystem : public MISystem
{
public:
	MResourceSystem();
	virtual ~MResourceSystem();

	template <typename TYPE>
	bool RegisterResourceType();

	template<typename TYPE>
	TYPE* CreateResource();

	template<typename TYPE>
	TYPE* CreateResource(const MString& strResourcePath);

	void SetSearchPath(const std::vector<MString>& vSearchPath);
	std::vector<MString> GetSearchPath() const { return m_vSearchPath; }

	const MType* GetResourceType(const MString& strResourcePath);

	MResource* LoadResource(const MString& strResourcePath, const MType* type = nullptr);
	void UnloadResource(MResource* pResource);
	
	void Reload(const MString& strResourcePath);

	MResource* FindResourceByID(const MResourceID& unID);

	std::map<MResourceID, MResource*>* GetAllResources() { return &m_tResources; }

	void MoveTo(MResource* pResource, const MString& strTargetPath);

private:

	std::map<const MType*, MResourceLoader*> m_tResourceLoader;

	std::map<MResourceID, MResource*> m_tResources;
	std::map<MString, MResource*> m_tPathResources;

	MIDPool<MResourceID> m_ResourceDB;
	std::map<MString, const MType*> m_tResSuffixToType;


	std::vector<MString> m_vSearchPath;

};

template <typename TYPE>
bool MResourceSystem::RegisterResourceType()
{
	if (!MTypeClass::IsType<TYPE, MResource>())
		return false;

	MString strTypeName = TYPE::GetResourceTypeName();
	std::vector<MString> vSuffixNameList = TYPE::GetSuffixList();

	MResourceLoader* pLoader = new MResourceLoaderTemp<TYPE>();
	for (const MString& suffix : vSuffixNameList)
	{
		if (m_tResSuffixToType.find(suffix) != m_tResSuffixToType.end())
		{
			GetEngine()->GetLogger()->Error("file suffix is already registed. suffix: %s", suffix);
			continue;
		}
		m_tResSuffixToType[suffix] = TYPE::GetClassType();
	}

	m_tResourceLoader[TYPE::GetClassType()] = pLoader;
	pLoader->m_strResourceTypeName = strTypeName;
	pLoader->m_vResourceSuffixList = vSuffixNameList;

	return true;
}

template<typename TYPE>
TYPE* MResourceSystem::CreateResource()
{
	if (!MTypeClass::IsType<TYPE, MResource>())
		return nullptr;

	TYPE* pResource = new TYPE();
	pResource->m_unResourceID = m_ResourceDB.GetNewID();
	pResource->m_pEngine = GetEngine();
	m_tResources[pResource->m_unResourceID] = pResource;

	pResource->OnCreated();
	return pResource;
}


template<typename TYPE>
TYPE* MResourceSystem::CreateResource(const MString& strResourcePath)
{
	if (MResource* pResource = m_tPathResources[strResourcePath])
		return dynamic_cast<TYPE*>(pResource);

	TYPE* pResource = CreateResource<TYPE>();
	pResource->m_strResourcePath = strResourcePath;
	m_tPathResources[strResourcePath] = pResource;

	return pResource;
}

#endif
