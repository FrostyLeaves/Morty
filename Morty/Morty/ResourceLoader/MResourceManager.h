/**
 * @File         MResourceManager
 * 
 * @Created      2019-08-11 13:40:09
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRESOURCEMANAGER_H_
#define _M_MRESOURCEMANAGER_H_
#include "MGlobal.h"
#include "MSingleInstance.h"
#include "MIDPool.h"
#include "MString.h"

#include <vector>
#include <map>

class MEngine;
class MIRenderer;
class MResource;
class MResourceLoader;

enum class MEResourceType
{
	Default = 0,
	Shader,
	Material,
	Texture,
	Node,
	Mesh,
	Skeleton,
	SkelAnim,
	RenderPass
};

class MORTY_API MResourceManager
{
public:
	MResourceManager();
	virtual ~MResourceManager();

	void SetOwnerEngine(MEngine* pEngine) { m_pEngine = pEngine; }

	template<typename Resource_TYPE>
	Resource_TYPE* CreateResource()
	{
		Resource_TYPE* pResource = new Resource_TYPE();
		if (!dynamic_cast<MResource*>(pResource))
		{
			delete pResource;
			return nullptr;
		}

		pResource->m_unResourceID = m_ResourceDB.GetNewID();
		pResource->m_pEngine = m_pEngine;
		m_tResources[pResource->m_unResourceID] = pResource;

		pResource->OnCreated();

		return pResource;

	}

	template<typename Resource_TYPE>
	Resource_TYPE* LoadVirtualResource(const MString& strResourcePath)
	{
		if (MResource* pResource = m_tPathResources[strResourcePath])
			return dynamic_cast<Resource_TYPE*>(pResource);

		Resource_TYPE* pResource = CreateResource<Resource_TYPE>();
		pResource->m_strResourcePath = strResourcePath;
		m_tPathResources[strResourcePath] = pResource;

		return pResource;
	}

	void SetSearchPath(const std::vector<MString>& vSearchPath);

	MEResourceType GetResourceType(const MString& strResourcePath);

	MResource* LoadResource(const MString& strResourcePath, const MEResourceType& eType = MEResourceType::Default);
	void UnloadResource(MResource* pResource);
	
	void Reload(const MString& strResourcePath);

	MResource* FindResourceByID(const MResourceID& unID);

	std::map<MResourceID, MResource*>* GetAllResources() { return &m_tResources; }

	void MoveTo(MResource* pResource, const MString& strTargetPath);

private:

	std::map<MEResourceType, MResourceLoader*> m_tResourceLoader;

	std::map<MResourceID, MResource*> m_tResources;
	std::map<MString, MResource*> m_tPathResources;

	MIDPool<MResourceID> m_ResourceDB;
	std::map<MString, MEResourceType> m_tResSuffixToType;

	MEngine* m_pEngine;


	std::vector<MString> m_vSearchPath;

};

#endif
