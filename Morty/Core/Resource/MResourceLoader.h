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
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

class MResourceSystem;
class MORTY_API MResourceLoader
{
public:
	MResourceLoader() = default;
	virtual ~MResourceLoader() = default;

public:

	virtual std::shared_ptr<MResource> Create(MResourceSystem* pManager) = 0;

	virtual std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath, const MString& svPath) = 0;

	MString strResourcePath;
	MString strResourceFullPath;

	std::shared_ptr<MResource> pResource = nullptr;
	std::unique_ptr<MResourceData> pResourceData = nullptr;
};

template<typename RESOURCE_TYPE, typename RESOURCE_DATA_TYPE>
class MORTY_API MResourceLoaderTemplate : public MResourceLoader
{
public:
	std::shared_ptr<MResource> Create(MResourceSystem* pManager) override
	{
		return pManager->CreateResource<RESOURCE_TYPE>();
	}

	std::unique_ptr<MResourceData> LoadResource(const MString& svFullPath, const MString& svPath) override
	{
		std::unique_ptr<RESOURCE_DATA_TYPE> pResourceData = std::make_unique<RESOURCE_DATA_TYPE>();

		std::vector<MByte> data;
		MFileHelper::ReadData(svFullPath, data);

		pResourceData->LoadBuffer(data);
		return pResourceData;
	}

};

#endif
