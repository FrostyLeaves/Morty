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

#define MTypedClassSign \
public: \
	static const class MTypeIdentifier* s_pTypeIdentifier; \
	virtual const class MTypeIdentifier* GetTypeIdentifier() { return s_pTypeIdentifier; }



#define MTypeIdentifierImplement(Class, BaseClass) \
    const class MTypeIdentifier* Class::s_pTypeIdentifier = new MTypeIdentifier(#Class, BaseClass::s_pTypeIdentifier);


class MORTY_CLASS MTypeIdentifier
{
public:
	MTypeIdentifier(const MString& strName, const MTypeIdentifier* pBaseTypeIdentifier) : m_strName(strName)
		, m_pBaseTypeIdentifier(pBaseTypeIdentifier)
		, m_unDeep(pBaseTypeIdentifier == nullptr ? 0 : pBaseTypeIdentifier->m_unDeep + 1) {}
	~MTypeIdentifier() {}

public:
	const MTypeIdentifier* GetBaseRTTI() const { return m_pBaseTypeIdentifier; }

public:
	const MString m_strName;
	const MTypeIdentifier* m_pBaseTypeIdentifier;
	const int m_unDeep;
private:

};

class MORTY_CLASS MTypedClass
{
public:
	MTypedClass() {};
	virtual ~MTypedClass() {}

	MTypedClassSign

	template <class T>
	T* DynamicCast()
	{
		if (nullptr == this) return nullptr;
		const MTypeIdentifier* pRTTI = GetTypeIdentifier();
		for (int i = pRTTI->m_unDeep - T::s_pTypeIdentifier->m_unDeep; i > 0; --i)
			pRTTI = pRTTI->m_pBaseTypeIdentifier;

		if (pRTTI == T::s_pTypeIdentifier)
			return (T*)(this);
		return nullptr;
	}
};


#endif
