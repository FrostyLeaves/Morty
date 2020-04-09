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

void JsonValueToMVariant(Value* pValue, MVariant& variant)
{
	if (pValue->IsString())
		variant = MVariant(MString(pValue->GetString()));
	else if (pValue->IsBool())
		variant = MVariant(pValue->GetBool());
	else if (pValue->IsInt())
		variant = MVariant(pValue->GetInt());
	else if (pValue->IsFloat())
		variant = MVariant(pValue->GetFloat());

	else if (pValue->IsObject())
	{
		MStruct sut;
		for (Value::MemberIterator iter = pValue->MemberBegin(); iter != pValue->MemberEnd(); ++iter)
		{
			MVariant child;
			JsonValueToMVariant(&(iter->value), child);
			sut.AppendMVariant(iter->name.GetString(), child);
		}
		variant = MVariant(sut);
	}

	else if (pValue->IsArray())
	{
		Value value = pValue->GetArray();
		MVariantArray sut;
		for (Value::MemberIterator iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter)
		{
			MVariant child;
			JsonValueToMVariant(&iter->value, child);
			sut.AppendMVariant(child);
		}
		variant = MVariant(sut);
	}

	else
		variant = MVariant();
}

void MVariantToJsonValue(const MVariant& var, Value* pValue, Document& doc)
{
	switch (var.GetType())
	{
	case MVariant::EBool:
		pValue->SetBool(var.IsTrue());
		break;

	case MVariant::EFloat:
		pValue->SetFloat(*var.GetFloat());
		break;

	case MVariant::EInt:
		pValue->SetInt(*var.GetInt());
		break;

	case MVariant::EString:
		pValue->SetString((*var.GetString()).c_str(), doc.GetAllocator());
		break;

	case MVariant::EStruct:
	{
		const MStruct* pStruct = var.GetStruct();
		for (unsigned int i = 0; i < pStruct->GetMemberCount(); ++i)
		{
			const MStruct::MStructMember* pMember = pStruct->GetMember(i);

			pValue->SetObject();

			Value name;
			name.SetString(pMember->strName.c_str(), doc.GetAllocator());
			Value value;
			MVariantToJsonValue(pMember->var, &value, doc);
			pValue->AddMember(name, value, doc.GetAllocator());

		}
		break;
	}

	case MVariant::EArray:
	{
		const MVariantArray* pArray = var.GetArray();
		for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
		{
			const MVariantArray::MStructMember* pMember = pArray->GetMember(i);

			pValue->SetArray();

			Value value;
			MVariantToJsonValue(pMember->var, &value, doc);

			pValue->PushBack(value, doc.GetAllocator());
		}
	}

	case MVariant::ENone:
	default:
		pValue->SetNull();
		break;
	}
}

void MJson::JsonToMVariant(const MString& strJson, MVariant& variant)
{
	Document doc;
	doc.Parse(strJson.c_str());

	if (doc.HasParseError())
	{
		MLogManager::GetInstance()->Error("Json Error Code : %d", doc.GetParseError());
		variant = MVariant();
		return;
	}

	 JsonValueToMVariant(&doc, variant);

}

void MJson::MVariantToJson(const MVariant& var, MString& strJson)
{
	Document doc;
	MVariantToJsonValue(var, &doc, doc);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);
	doc.Accept(writer);

	strJson = buffer.GetString();
}
