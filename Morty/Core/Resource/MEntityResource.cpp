#include "Resource/MEntityResource.h"

#include "Utility/MJson.h"
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
	return MFileHelper::ReadString(strResourcePath, m_entityData);
}

bool MEntityResource::SaveTo(const MString& strResourcePath)
{
	return MFileHelper::WriteString(strResourcePath, m_entityData);
}
