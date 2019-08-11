#include "MModelLoader.h"
#include "MResourceManager.h"
#include "MModelResource.h"

MModelLoader::MModelLoader()
: MResourceLoader()
{
    
}

MModelLoader::~MModelLoader()
{
    
}

MResource* MModelLoader::Load(const MString& svPath)
{
    MModelResource* pResource = MResourceManager::GetInstance()->CreateResource<MModelResource>();
	if (pResource->Load(svPath))
		return pResource;

	delete pResource;
	return nullptr;

}
