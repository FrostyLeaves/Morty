﻿/**
 * @File         MResourceLoader
 * 
 * @Created      2019-08-06 17:59:45
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRESOURCELOADER_H_
#define _M_MRESOURCELOADER_H_
#include "MGlobal.h"
#include "MString.h"

class MResource;
class MResourceManager;
class MORTY_API MResourceLoader
{
public:
	MResourceLoader() {}
	virtual ~MResourceLoader() {}

public:

	virtual MResource* Create(MResourceManager* pManager) = 0;
	virtual MResource* Load(MResourceManager* pManager, const MString& svPath) = 0;

protected:

	bool ResourceLoad(MResource* pResource, const MString& svPath)
	{
		pResource->m_strResourcePath = svPath;
		if (pResource->Load(svPath))
			return true;

		pResource->m_strResourcePath = "";
		return false;
	}

};

template <typename RESOURCE_TYPE>
class MORTY_API MResourceLoaderTemp : public MResourceLoader
{
public:
	MResourceLoaderTemp() {}
	virtual ~MResourceLoaderTemp() {}

public:

	virtual MResource* Create(MResourceManager* pManager) override
	{
		return pManager->CreateResource<RESOURCE_TYPE>();
	}

	virtual MResource* Load(MResourceManager* pManager, const MString& svPath) override
	{
		RESOURCE_TYPE* pResource = pManager->CreateResource<RESOURCE_TYPE>();
		if (pResource)
		{
			if (ResourceLoad(pResource, svPath))
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
