/**
 * @File         MResourceManager
 * 
 * @Created      2019-08-11 13:40:09
 *
 * @Author       Morty
**/

#ifndef _M_MRESOURCEMANAGER_H_
#define _M_MRESOURCEMANAGER_H_
#include "MGlobal.h"
#include "MSingleInstance.h"
#include "MIDPool.h"
#include "MString.h"

#include <map>

class MEngine;
class MIRenderer;
class MResource;
class MResourceLoader;
class MORTY_CLASS MResourceManager
{
public:
	enum MEResourceType
	{
		Default = 0,
		Model = 1,
		Shader = 2,
		Material = 3,
		Texture = 4,
	};

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

		pResource->m_unResourceID = m_pResourceDB->GetNewID();
		pResource->m_pEngine = m_pEngine;

		m_tIDResources[pResource->m_unResourceID] = pResource;

		return pResource;
	}

	MEResourceType GetResourceType(const MString& strResourcePath);

	MResource* Load(const MString& strResourcePath, const MEResourceType& eType = MEResourceType::Default);
	MResource* Create(const MEResourceType& eType);
	void Reload(const MString& strResourcePath);

private:

	std::map<MEResourceType, MResourceLoader*> m_tResourceLoader;

	std::map<MResourceID, MResource*> m_tIDResources;
	std::map<MString, MResource*> m_tPathResources;

	MIDPool<MResourceID>* m_pResourceDB;
	std::map<MString, MEResourceType> m_tResSuffixToType;

	MEngine* m_pEngine;
};



#endif
