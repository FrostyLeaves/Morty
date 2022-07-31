#include "Type/MType.h"

const MType* MTypeClass::GetClassType()
{
	static const MType type("MTypeClass", nullptr);
	return &type;
}

MTypeClass* MTypeClass::New(const MString& strTypeName)
{
	auto findResult = GetFactory().find(strTypeName);
	if(findResult != GetFactory().end())
		return findResult->second.m_funcNew();

	return nullptr;
}

const MType* MTypeClass::GetType(const MString& strTypeName)
{
	auto findResult = GetFactory().find(strTypeName);
	if (findResult != GetFactory().end())
		return findResult->second.m_pType;

	return nullptr;
}

std::map<MString, MDynamicTypeInfo>& MTypeClass::GetFactory()
{
	static std::map<MString, MDynamicTypeInfo> m;
	return m;
}

