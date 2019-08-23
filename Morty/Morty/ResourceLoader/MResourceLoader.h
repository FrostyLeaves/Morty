/**
 * @File         MResourceLoader
 * 
 * @Created      2019-08-06 17:59:45
 *
 * @Author       Morty
**/

#ifndef _M_MRESOURCELOADER_H_
#define _M_MRESOURCELOADER_H_
#include "MGlobal.h"
#include "MString.h"

class MResource;
class MResourceManager;
class MORTY_CLASS MResourceLoader
{
public:
	MResourceLoader() {}
	virtual ~MResourceLoader() {}

public:

	virtual MResource* Load(MResourceManager* pManager, const MString& svPath) = 0;

protected:

	bool ResourceLoad(MResource* pResource, const MString& svPath)
	{
		return pResource->Load(svPath);
	}

};

template <typename RESOURCE_TYPE>
class MORTY_CLASS MResourceLoaderTemp : public MResourceLoader
{
public:
	MResourceLoaderTemp() {}
	virtual ~MResourceLoaderTemp() {}

public:

	virtual MResource* Load(MResourceManager* pManager, const MString& svPath)
	{
		RESOURCE_TYPE* pResource = pManager->CreateResource<RESOURCE_TYPE>();
		if (pResource)
		{
			if (ResourceLoad(pResource, svPath))
				return pResource;
			delete pResource;
		}
		return nullptr;
	}
	
private:

};


#endif
