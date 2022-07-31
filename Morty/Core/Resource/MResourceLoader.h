/**
 * @File         MResourceLoader
 * 
 * @Created      2019-08-06 17:59:45
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRESOURCELOADER_H_
#define _M_MRESOURCELOADER_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Resource/MResource.h"

class MResourceSystem;
class MORTY_API MResourceLoader
{
public:
	MResourceLoader() {}
	virtual ~MResourceLoader() {}

public:

	virtual std::shared_ptr<MResource> Create(MResourceSystem* pManager) = 0;
	virtual std::shared_ptr<MResource> Load(MResourceSystem* pManager, const MString& svFullPath, const MString& svPath) = 0;

protected:

	bool ResourceLoad(std::shared_ptr<MResource> pResource, const MString& svPath)
	{
		if (pResource->Load(svPath))
			return true;

		return false;
	}

	void SetResourcePath(std::shared_ptr<MResource> pResource, const MString& svPath)
	{
		pResource->m_strResourcePath = svPath;
	}



private:

	friend class MResourceSystem;
	MString m_strResourceTypeName;
	std::vector<MString> m_vResourceSuffixList;
};

template <typename RESOURCE_TYPE>
class MORTY_API MResourceLoaderTemp : public MResourceLoader
{
public:
	MResourceLoaderTemp() {}
	virtual ~MResourceLoaderTemp() {}

public:

	virtual std::shared_ptr<MResource> Create(MResourceSystem* pManager) override
	{
		return pManager->CreateResource<RESOURCE_TYPE>();
	}

	virtual std::shared_ptr<MResource> Load(MResourceSystem* pManager, const MString& svFullPath, const MString& svPath) override
	{
		std::shared_ptr<RESOURCE_TYPE> pResource = pManager->CreateResource<RESOURCE_TYPE>();
		if (pResource)
		{
			SetResourcePath(pResource, svPath);
			if (ResourceLoad(pResource, svFullPath))
			{
				return pResource;
			}
			pManager->UnloadResource(pResource);
		}
		return nullptr;
	}
	
private:

};


#endif
