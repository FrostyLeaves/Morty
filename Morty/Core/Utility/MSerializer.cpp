#include "MSerializer.h"
#include "MJson.h"



void MSerializer::Encode(MString& strCode)
{
	MVariant var = MStruct();
	MStruct* pStruct = var.GetStruct();

	WriteToStruct(*pStruct);

	MJson::MVariantToJson(var, strCode);
}

bool MSerializer::Decode(MString& strCode)
{
	MVariant var;
	MJson::JsonToMVariant(strCode, var);

	ReadFromStruct(*var.GetStruct());
	return true;
}
