#include "Resource/MEntityResource.h"

#include "Utility/MFileHelper.h"

MORTY_CLASS_IMPLEMENT(MEntityResource, MResource)

MEntityResource::MEntityResource()
{

}

MEntityResource::~MEntityResource()
{

}

bool MEntityResource::Load(const MString& strResourcePath)
{
	return MFileHelper::ReadData(strResourcePath, m_entityData);
}

bool MEntityResource::SaveTo(const MString& strResourcePath)
{
	return MFileHelper::WriteData(strResourcePath, m_entityData);
}
