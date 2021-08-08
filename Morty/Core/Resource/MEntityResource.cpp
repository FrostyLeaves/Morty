#include "MEntityResource.h"

#include "MJson.h"
#include "MFileHelper.h"

MORTY_CLASS_IMPLEMENT(MEntityResource, MResource)

MEntityResource::MEntityResource()
{

}

MEntityResource::~MEntityResource()
{

}

bool MEntityResource::Load(const MString& strResourcePath)
{
	MString strCode;
	if (!MFileHelper::ReadString(strResourcePath, strCode))
		return false;

	return MJson::JsonToMVariant(strCode, m_entityStruct);
}

bool MEntityResource::SaveTo(const MString& strResourcePath)
{
	MString strCode;

	MJson::MVariantToJson(m_entityStruct, strCode);

	return MFileHelper::WriteString(strResourcePath, strCode);
}
