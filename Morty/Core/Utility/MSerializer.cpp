#include "MSerializer.h"
#include "MJson.h"

MORTY_CLASS_IMPLEMENT(MSerializer, MTypeClass)


MSerializer::MSerializer()
	: m_unReferenceIdx(MGlobal::M_INVALID_INDEX)
{

}

void MSerializer::WriteToStruct(MStruct& srt)
{
}

void MSerializer::ReadFromStruct(const MStruct& srt)
{
}

void MSerializer::Encode(MString& strCode)
{
	MVariant var = MStruct();
	MStruct* pStruct = var.GetStruct();

	Encode(*pStruct);

	MJson::MVariantToJson(var, strCode);
}

void MSerializer::Encode(MStruct& srt)
{
	WriteToStruct(srt);
}

bool MSerializer::Decode(const MStruct& srt)
{
	ReadFromStruct(srt);
	return true;
}

bool MSerializer::Decode(MString& strCode)
{
	MVariant var;
	MJson::JsonToMVariant(strCode, var);

	return Decode(*var.GetStruct());
}
