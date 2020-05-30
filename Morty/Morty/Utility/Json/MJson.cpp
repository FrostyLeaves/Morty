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

#define FIND_MEMBER(VAR, PARENT, NAME, TYPENAME, DEFAULT) { \
	Value::MemberIterator iter = PARENT->FindMember(NAME);	\
	if(iter != PARENT->MemberEnd())	\
		VAR = iter->value.Get##TYPENAME();	\
	else VAR = DEFAULT; \
}

#define ADD_MEMBER(VAR, NAME, DOC) {|

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
		Value::MemberIterator type = pValue->FindMember("<T>");
		if (type != pValue->MemberEnd())
		{
			int eType(type->value.GetInt());
			switch (eType)
			{
			case MVariant::EVector3:
			{
				Vector3 var;
				FIND_MEMBER(var.x, pValue, "x", Float, 0.0f);
				FIND_MEMBER(var.y, pValue, "y", Float, 0.0f);
				FIND_MEMBER(var.z, pValue, "z", Float, 0.0f);
				variant = var;
				break;
			}
			case MVariant::EVector4:
			{
				Vector4 var;
				FIND_MEMBER(var.x, pValue, "x", Float, 0.0f);
				FIND_MEMBER(var.y, pValue, "y", Float, 0.0f);
				FIND_MEMBER(var.z, pValue, "z", Float, 0.0f);
				FIND_MEMBER(var.w, pValue, "w", Float, 0.0f);
				variant = var;
				break;
			}

			case MVariant::EQuaternion:
			{
				Quaternion quat;
				FIND_MEMBER(quat.w, pValue, "w", Float, 0.0f);
				FIND_MEMBER(quat.x, pValue, "x", Float, 0.0f);
				FIND_MEMBER(quat.y, pValue, "y", Float, 0.0f);
				FIND_MEMBER(quat.z, pValue, "z", Float, 0.0f);
				variant = quat;
				break;
			}

			case MVariant::EMatrix3:
			{
				Matrix3 mat;
				FIND_MEMBER(mat.m[0][0], pValue, "00", Float, 0.0f);
				FIND_MEMBER(mat.m[0][1], pValue, "01", Float, 0.0f);
				FIND_MEMBER(mat.m[0][2], pValue, "02", Float, 0.0f);

				FIND_MEMBER(mat.m[1][0], pValue, "10", Float, 0.0f);
				FIND_MEMBER(mat.m[1][1], pValue, "11", Float, 0.0f);
				FIND_MEMBER(mat.m[1][2], pValue, "12", Float, 0.0f);

				FIND_MEMBER(mat.m[2][0], pValue, "20", Float, 0.0f);
				FIND_MEMBER(mat.m[2][1], pValue, "21", Float, 0.0f);
				FIND_MEMBER(mat.m[2][2], pValue, "22", Float, 0.0f);
				variant = mat;
				break;
			}
			case MVariant::EMatrix4:
			{
				Matrix3 mat;
				FIND_MEMBER(mat.m[0][0], pValue, "00", Float, 0.0f);
				FIND_MEMBER(mat.m[0][1], pValue, "01", Float, 0.0f);
				FIND_MEMBER(mat.m[0][2], pValue, "02", Float, 0.0f);
				FIND_MEMBER(mat.m[0][3], pValue, "03", Float, 0.0f);

				FIND_MEMBER(mat.m[1][0], pValue, "10", Float, 0.0f);
				FIND_MEMBER(mat.m[1][1], pValue, "11", Float, 0.0f);
				FIND_MEMBER(mat.m[1][2], pValue, "12", Float, 0.0f);
				FIND_MEMBER(mat.m[1][3], pValue, "13", Float, 0.0f);

				FIND_MEMBER(mat.m[2][0], pValue, "20", Float, 0.0f);
				FIND_MEMBER(mat.m[2][1], pValue, "21", Float, 0.0f);
				FIND_MEMBER(mat.m[2][2], pValue, "22", Float, 0.0f);
				FIND_MEMBER(mat.m[2][3], pValue, "23", Float, 0.0f);

				FIND_MEMBER(mat.m[3][0], pValue, "30", Float, 0.0f);
				FIND_MEMBER(mat.m[3][1], pValue, "31", Float, 0.0f);
				FIND_MEMBER(mat.m[3][2], pValue, "32", Float, 0.0f);
				FIND_MEMBER(mat.m[3][3], pValue, "33", Float, 0.0f);
				variant = mat;
				break;
			}

			default:
				variant = MVariant();
				break;
			}
		}
		else
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
	}

	else if (pValue->IsArray())
	{
		Value value = pValue->GetArray();
		unsigned int unSize = value.Size();
		MVariantArray sut;
		for (unsigned int i = 0; i < unSize; ++i)
		{
			MVariant child;
			JsonValueToMVariant(&value[i], child);
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
		pValue->SetFloat(var.GetFloat());
		break;

	case MVariant::EInt:
		pValue->SetInt(var.GetInt());
		break;

	case MVariant::EString:
		pValue->SetString((var.GetString()).c_str(), doc.GetAllocator());
		break;

	case MVariant::EVector3:
	{
		float* vFloat = var.CastFloatUnsafe();
		pValue->SetObject();
		pValue->AddMember("<T>", MVariant::EVector3, doc.GetAllocator());

		pValue->AddMember("x", vFloat[0], doc.GetAllocator());
		pValue->AddMember("y", vFloat[1], doc.GetAllocator());
		pValue->AddMember("z", vFloat[2], doc.GetAllocator());

		break;
	}

	case MVariant::EVector4:
	{
		float* vFloat = var.CastFloatUnsafe();
		pValue->SetObject();
		pValue->AddMember("<T>", MVariant::EVector4, doc.GetAllocator());

		pValue->AddMember("x", vFloat[0], doc.GetAllocator());
		pValue->AddMember("y", vFloat[1], doc.GetAllocator());
		pValue->AddMember("z", vFloat[2], doc.GetAllocator());
		pValue->AddMember("w", vFloat[3], doc.GetAllocator());

		break;
	}

	case MVariant::EQuaternion:
	{
		float* vFloat = var.CastFloatUnsafe();
		pValue->SetObject();
		pValue->AddMember("<T>", MVariant::EQuaternion, doc.GetAllocator());

		pValue->AddMember("w", vFloat[0], doc.GetAllocator());
		pValue->AddMember("x", vFloat[1], doc.GetAllocator());
		pValue->AddMember("y", vFloat[2], doc.GetAllocator());
		pValue->AddMember("z", vFloat[3], doc.GetAllocator());

		break;
	}

	case MVariant::EMatrix3:
	{
		float* vFloat = var.CastFloatUnsafe();
		pValue->SetObject();
		pValue->AddMember("<T>", MVariant::EMatrix3, doc.GetAllocator());

		pValue->AddMember("00", vFloat[0], doc.GetAllocator());
		pValue->AddMember("01", vFloat[1], doc.GetAllocator());
		pValue->AddMember("02", vFloat[2], doc.GetAllocator());

		pValue->AddMember("10", vFloat[3], doc.GetAllocator());
		pValue->AddMember("11", vFloat[4], doc.GetAllocator());
		pValue->AddMember("12", vFloat[5], doc.GetAllocator());

		pValue->AddMember("20", vFloat[6], doc.GetAllocator());
		pValue->AddMember("21", vFloat[7], doc.GetAllocator());
		pValue->AddMember("22", vFloat[8], doc.GetAllocator());

		break;
	}

	case MVariant::EMatrix4:
	{
		float* vFloat = var.CastFloatUnsafe();
		pValue->SetObject();
		pValue->AddMember("<T>", MVariant::EMatrix4, doc.GetAllocator());

		pValue->AddMember("00", vFloat[0], doc.GetAllocator());
		pValue->AddMember("01", vFloat[1], doc.GetAllocator());
		pValue->AddMember("02", vFloat[2], doc.GetAllocator());
		pValue->AddMember("03", vFloat[3], doc.GetAllocator());

		pValue->AddMember("10", vFloat[4], doc.GetAllocator());
		pValue->AddMember("11", vFloat[5], doc.GetAllocator());
		pValue->AddMember("12", vFloat[6], doc.GetAllocator());
		pValue->AddMember("13", vFloat[7], doc.GetAllocator());

		pValue->AddMember("20", vFloat[8], doc.GetAllocator());
		pValue->AddMember("21", vFloat[9], doc.GetAllocator());
		pValue->AddMember("22", vFloat[10], doc.GetAllocator());
		pValue->AddMember("23", vFloat[11], doc.GetAllocator());

		pValue->AddMember("30", vFloat[12], doc.GetAllocator());
		pValue->AddMember("31", vFloat[13], doc.GetAllocator());
		pValue->AddMember("32", vFloat[14], doc.GetAllocator());
		pValue->AddMember("33", vFloat[15], doc.GetAllocator());

		break;
	}

	case MVariant::EStruct:
	{
		const MStruct* pStruct = var.GetStruct();
		pValue->SetObject();
		for (unsigned int i = 0; i < pStruct->GetMemberCount(); ++i)
		{
			const MStruct::MStructMember* pMember = pStruct->GetMember(i);


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
		pValue->SetArray();
		for (unsigned int i = 0; i < pArray->GetMemberCount(); ++i)
		{
			const MVariantArray::MStructMember* pMember = pArray->GetMember(i);


			Value value;
			MVariantToJsonValue(pMember->var, &value, doc);
			pValue->PushBack(value, doc.GetAllocator());

		}

		break;
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
