#include "Type/MType.h"

const MType* MTypeClass::GetClassType()
{
	static const MType type("MTypeClass", nullptr);
	return &type;
}

MTypeClass* MTypeClass::New(const MString& strTypeName)
{
	auto findResult = GetNameTable().find(strTypeName);
	if(findResult != GetNameTable().end())
		return findResult->second.m_funcNew();

	return nullptr;
}

MTypeClass* MTypeClass::New(const MType* type)
{
	auto findResult = GetTypeTable().find(type);
	if (findResult != GetTypeTable().end())
		return findResult->second.m_funcNew();

	return nullptr;
}

const MType* MTypeClass::GetType(const MString& strTypeName)
{
	auto findResult = GetNameTable().find(strTypeName);
	if (findResult != GetNameTable().end())
		return findResult->second.m_pType;

	return nullptr;
}

std::map<MString, MDynamicTypeInfo>& MTypeClass::GetNameTable()
{
	static std::map<MString, MDynamicTypeInfo> m;
	return m;
}

std::map<const MType*, MDynamicTypeInfo>& MTypeClass::GetTypeTable()
{
	static std::map<const MType*, MDynamicTypeInfo> m;
	return m;
}


