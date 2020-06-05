/**
 * @File         MSerializer
 * 
 * @Created      2020-05-29 16:30:19
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSERIALIZER_H_
#define _M_MSERIALIZER_H_
#include "MGlobal.h"
#include "MVariant.h"

class MORTY_CLASS MSerializer
{


public:
	virtual void WriteToStruct(MStruct& srt) {};
	virtual void ReadFromStruct(MStruct& srt) {};

	void Encode(MString& strCode);
	bool Decode(MString& strCode);

	template <typename T>
	static T* FindWriteVariant(MStruct& srt, const MString& strName);

	template <typename T>
	static T* FindReadVariant(MStruct& srt, const MString& strName);
private:

};

template <typename T>
T* MSerializer::FindWriteVariant(MStruct& srt, const MString& strName)
{
	MVariant* p = srt.AppendMVariant(strName, T());
	return p->GetPointerUnsafe<T>();
}

template <typename T>
T* MSerializer::FindReadVariant(MStruct& srt, const MString& strName)
{
	MVariant* pVariant = srt.FindMember(strName);
	if (nullptr == pVariant)
		return nullptr;

	return pVariant->GetPointerUnsafe<T>();
}

#define M_SERIALIZER_BEGIN( STATE ) if(MStruct* pStruct = Find##STATE##Variant<MStruct>(srt, Class::GetClassTypeName())) {
#define M_SERIALIZER_END }

#define M_SERIALIZER_WRITE_VALUE( NAME, GET_FUNC) \
	pStruct->AppendMVariant(NAME, GET_FUNC());

#define M_SERIALIZER_READ_VALUE( NAME, SET_FUNC, TYPE) \
	if(MVariant* pVariant = pStruct->FindMember(NAME)) \
		SET_FUNC(pVariant->Get##TYPE());
#endif
