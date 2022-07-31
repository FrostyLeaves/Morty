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
#include "Utility/MVariant.h"

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

	template <typename T>
	static T* FindWriteVariant(MStruct& srt, const MString& strName);

	template <typename T>
	static const T* FindReadVariant(const MStruct& srt, const MString& strName);
private:

	uint32_t m_unReferenceIdx;
};

template <typename T>
T* MSerializer::FindWriteVariant(MStruct& srt, const MString& strName)
{
	uint32_t unIndex = srt.AppendMVariant(strName, T());
	return srt.GetMember(unIndex)->var.GetPointerUnsafe<T>();
}

template <typename T>
const T* MSerializer::FindReadVariant(const MStruct& srt, const MString& strName)
{
	const MVariant* pVariant = srt.FindMember(strName);
	if (nullptr == pVariant)
		return nullptr;

	return pVariant->GetPointerUnsafe<T>();
}

#define M_SERIALIZER_READ_BEGIN if(const MStruct* pStruct = MSerializer::FindReadVariant<MStruct>(srt, Class::GetClassTypeName())) {
#define M_SERIALIZER_WRITE_BEGIN if(MStruct* pStruct = MSerializer::FindWriteVariant<MStruct>(srt, Class::GetClassTypeName())) {
#define M_SERIALIZER_END }

#define M_SERIALIZER_WRITE_VALUE( NAME, GET_FUNC) \
	pStruct->AppendMVariant(NAME, GET_FUNC());

#define M_SERIALIZER_READ_VALUE( NAME, SET_FUNC, TYPE) \
	if(const MVariant* pVariant = pStruct->FindMember(NAME)) \
		if(auto pValue = pVariant->Get##TYPE()) \
			SET_FUNC(*pValue);

#endif
