#include "MTypedClass.h"

MTypeIdentifierConstPointer MTypedClass::GetClassTypeIdentifier()
{
	static const MTypeIdentifier* pTypeIdentifier = new MTypeIdentifier("MTypedClass", nullptr);
	return pTypeIdentifier;
}

MTypedClass* MTypedClass::New(const MString& strTypeName)
{
	if(std::function<MTypedClass* (void)> func = GetFactory()[strTypeName])
		return func();
	return nullptr;
}

std::map<MString, std::function<MTypedClass* (void)>>& MTypedClass::GetFactory()
{
	static std::map<MString, std::function<MTypedClass* (void)>> m;
	return m;
}

