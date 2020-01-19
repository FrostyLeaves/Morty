#include "MTypedClass.h"

MTypeIdentifierConstPointer MTypedClass::GetClassTypeIdentifier()
{
	static const MTypeIdentifier* pTypeIdentifier = new MTypeIdentifier("MTypedClass", nullptr);
	return pTypeIdentifier;
}
