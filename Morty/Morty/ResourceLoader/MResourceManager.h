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

class MResource;
class MResourceLoader;
class MORTY_CLASS MResourceManager
{
public:
	enum MEResourceType
	{
		Model = 1,
	};

public:
	MResourceManager();
	virtual ~MResourceManager();

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

		m_tIDResources[pResource->m_unResourceID] = pResource;

		return pResource;
	}

	MEResourceType GetResourceType(const MString& strResourcePath);

	MResource* Load(const MString& strResourcePath);
	void Reload(const MString& strResourcePath);

private:

	std::map<MEResourceType, MResourceLoader*> m_tResourceLoader;

	std::map<MResourceID, MResource*> m_tIDResources;
	std::map<MString, MResource*> m_tPathResources;

	MIDPool<MResourceID>* m_pResourceDB;
};



#endif
