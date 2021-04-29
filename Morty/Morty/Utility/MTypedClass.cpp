#include "MTypedClass.h"

MTypeIdentifierConstPointer MTypedClass::GetClassTypeIdentifier()
{
	static const MTypeIdentifier* pTypeIdentifier = new MTypeIdentifier("MTypedClass", nullptr);
	return pTypeIdentifier;
}

MTypedClass* MTypedClass::New(const MString& strTypeName)
{
	auto findResult = GetFactory().find(strTypeName);
	if(findResult != GetFactory().end())
		return findResult->second.m_funcNew();

	return nullptr;
}

MTypeIdentifierConstPointer MTypedClass::GetType(const MString& strTypeName)
{
	auto findResult = GetFactory().find(strTypeName);
	if (findResult != GetFactory().end())
		return findResult->second.m_pType;

	return nullptr;
}

std::map<MString, MDynamicTypeInfo>& MTypedClass::GetFactory()
{
	static std::map<MString, MDynamicTypeInfo> m;
	return m;
}

