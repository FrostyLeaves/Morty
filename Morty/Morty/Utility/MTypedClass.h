/**
 * @File         MTypedClass
 * 
 * @Created      2019-12-27 19:21:39
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTYPEDCLASS_H_
#define _M_MTYPEDCLASS_H_
#include "MGlobal.h"
#include "MString.h"

#define MTypedClassSign(Class) \
public: \
	static MTypeIdentifierConstPointer GetClassTypeIdentifier(); \
	virtual MTypeIdentifierConstPointer GetTypeIdentifier() { return Class::GetClassTypeIdentifier(); };



#define MTypeIdentifierImplement(Class, BaseClass) \
typedef BaseClass Super; \
    MTypeIdentifierConstPointer Class::GetClassTypeIdentifier() { \
		static const MTypeIdentifier typeIdentifier(#Class, BaseClass::GetClassTypeIdentifier()); \
		return &typeIdentifier; \
	} \


class MTypeIdentifier
{
public:
	MTypeIdentifier(const MString& strName, const MTypeIdentifier* pBaseTypeIdentifier) : m_strName(strName)
		, m_pBaseTypeIdentifier(pBaseTypeIdentifier)
		, m_unDeep(pBaseTypeIdentifier == nullptr ? 0 : pBaseTypeIdentifier->m_unDeep + 1) {}
	~MTypeIdentifier() {}

public:
	const MString m_strName;
	const MTypeIdentifier* m_pBaseTypeIdentifier;
	const int m_unDeep;
private:

};

typedef const MTypeIdentifier* MTypeIdentifierConstPointer;

class MTypedClass
{
public:
	MTypedClass() {};
	virtual ~MTypedClass() {}

	MTypedClassSign(MTypedClass)

		template <class T>
	T* DynamicCast()
	{
		if (nullptr == this) return nullptr;
		MTypeIdentifierConstPointer pTypeIdent = GetTypeIdentifier();
		MTypeIdentifierConstPointer pClassIdent = T::GetClassTypeIdentifier();
		for (int i = pTypeIdent->m_unDeep - pClassIdent->m_unDeep; i > 0; --i)
			pTypeIdent = pTypeIdent->m_pBaseTypeIdentifier;

		if (pTypeIdent == pClassIdent)
			return (T*)(this);
		return nullptr;
	}

	template<typename T1, typename T2>
	static bool IsType()
	{
		MTypeIdentifierConstPointer pTypeIdent = T1::GetClassTypeIdentifier();
		MTypeIdentifierConstPointer pClassIdent = T2::GetClassTypeIdentifier();
		for (int i = pTypeIdent->m_unDeep - pClassIdent->m_unDeep; i > 0; --i)
			pTypeIdent = pTypeIdent->m_pBaseTypeIdentifier;

		return pTypeIdent == pClassIdent;
	}
};


#endif
