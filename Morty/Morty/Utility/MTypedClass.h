/**
 * @File         MTypedClass
 * 
 * @Created      2019-12-27 19:21:39
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTYPEDCLASS_H_
#define _M_MTYPEDCLASS_H_
#include "MGlobal.h"
#include "MString.h"

#include <map>
#include <functional>

#define MTypedInterfaceSign(Class) \
public: \
static MTypeIdentifierConstPointer GetClassTypeIdentifier(); \
static MString GetClassTypeName(){ return Class::GetClassTypeIdentifier()->m_strName;}\
virtual MTypeIdentifierConstPointer GetTypeIdentifier() { return Class::GetClassTypeIdentifier(); }; \


#define MTypedInterfaceImplement(CurrClass, BaseClass) \
typedef BaseClass Super; \
typedef CurrClass Class;\
    MTypeIdentifierConstPointer Class::GetClassTypeIdentifier() { \
		static const MTypeIdentifier typeIdentifier(#CurrClass, BaseClass::GetClassTypeIdentifier()); \
		return &typeIdentifier; \
	} \


#define MTypedCreatorSign(Class) \
public: \
	static MTypedClass* New() { return new Class(); } \
	static MTypedCreator<Class> s_##Class##Creator; \


#define MTypedCreatorImplement(Class) \
	MTypedCreator<Class> Class::s_##Class##Creator = MTypedCreator<Class>();\


#define MTypedClassSign(Class) \
MTypedInterfaceSign(Class) \
MTypedCreatorSign(Class) \


#define MTypedClassImplement(Class, BaseClass) \
MTypedInterfaceImplement(Class, BaseClass)\
MTypedCreatorImplement(Class)\


class MTypeIdentifier
{
public:
	MTypeIdentifier(const MString& strName, const MTypeIdentifier* pBaseTypeIdentifier) : m_strName(strName)
		, m_pBaseTypeIdentifier(pBaseTypeIdentifier)
		, m_unDeep(pBaseTypeIdentifier == nullptr ? 0 : pBaseTypeIdentifier->m_unDeep + 1) {
	}
	~MTypeIdentifier() {}

public:
	const MString m_strName;
	const MTypeIdentifier* m_pBaseTypeIdentifier;
	const int m_unDeep;
};

template <typename T>
class MTypedCreator
{
public:
	MTypedCreator<T>();
};

typedef const MTypeIdentifier* MTypeIdentifierConstPointer;

class MORTY_API MTypedClass
{
public:
	MTypedClass() {};
	virtual ~MTypedClass() {}


	MString GetTypeName() {
			return GetTypeIdentifier()->m_strName;
	}

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

	template<typename T>
	static void RegisterTypedClass() {
		const MString& strName = T::GetClassTypeIdentifier()->m_strName;
		GetFactory()[strName] = &T::New;
	}

	static MTypedClass* New(const MString& strTypeName);

	static std::map<MString, std::function<MTypedClass* (void)> >& GetFactory();
};

template <typename T>
MTypedCreator<T>::MTypedCreator()
{
	MTypedClass::RegisterTypedClass<T>();
}

#endif
