#include "Utility/MSerializer.h"

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
}

void MSerializer::Encode(MStruct& srt)
{
}

bool MSerializer::Decode(const MStruct& srt)
{
	return false;
}

bool MSerializer::Decode(MString& strCode)
{
	return false;
}
