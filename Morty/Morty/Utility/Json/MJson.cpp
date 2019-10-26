#include "MJson.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"

#include "MVariant.h"
#include "MLogManager.h"

using namespace rapidjson;

MJson::MJson()
{

}

MJson::~MJson()
{

}

MVariant JsonValueToMVariant(Value* pValue)
{
	if (pValue->IsNull())
		return MVariant();
	if (pValue->IsString())
		return MVariant(MString(pValue->GetString()));
	if (pValue->IsBool())
		return MVariant(pValue->GetBool());
	if (pValue->IsInt())
		return MVariant(pValue->GetInt());
	if (pValue->IsFloat())
		return MVariant(pValue->GetFloat());

	if (pValue->IsObject())
	{
		MStruct sut;
		for (Value::MemberIterator iter = pValue->MemberBegin(); iter != pValue->MemberEnd(); ++iter)
			sut.AppendMVariant(iter->name.GetString(), JsonValueToMVariant(&(iter->value)));
		return MVariant(sut);
	}

	if (pValue->IsArray())
	{
		Value value = pValue->GetArray();
		MVariantArray sut;
		for (Value::MemberIterator iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter)
			sut.AppendMVariant(JsonValueToMVariant(&iter->value));
		return MVariant(sut);
	}

	return MVariant();
}

MVariant MJson::JsonToMVariant(const MString& strJson)
{
	Document doc;
	doc.Parse(strJson.c_str());

	if (doc.HasParseError())
	{
		MLogManager::GetInstance()->Error("Json Error Code : %d", doc.GetParseError());
		return MVariant();
	}

	return JsonValueToMVariant(&doc);

}
