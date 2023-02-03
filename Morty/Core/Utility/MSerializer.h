/**
 * @File         MSerializer
 * 
 * @Created      2020-05-29 16:30:19
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSERIALIZER_H_
#define _M_MSERIALIZER_H_
#include "Utility/MGlobal.h"
#include "Type/MType.h"
#include "Variant/MVariant.h"

class MORTY_API MSerializer : public MTypeClass
{
	MORTY_CLASS(MSerializer)
public:
	MSerializer();

public:
	virtual void WriteToStruct(MStruct& srt);
	virtual void ReadFromStruct(const MStruct& srt);

	void Encode(MString& strCode);
	bool Decode(MString& strCode);

	void Encode(MStruct& srt);
	bool Decode(const MStruct& srt);

private:

	uint32_t m_unReferenceIdx;
};

#endif
