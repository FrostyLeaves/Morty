/**
 * @File         MTypeClass
 * 
 * @Created      2019-12-27 19:21:39
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#define M_TYPE_DECL(Class) \
public: \
static const MType* GetClassType(); \
static MString GetClassTypeName(){ return Class::GetClassType()->m_strName;}\
const MType* GetType() const override { return Class::GetClassType(); }; \


#define M_TYPE_IMPLEMENT(CurrClass, BaseClass) \
typedef BaseClass Super; \
typedef CurrClass Class;\
    const MType* Class::GetClassType() { \
		static const MType type(#CurrClass, BaseClass::GetClassType()); \
		return &type; \
	} \


#define M_TYPE_CREATE_DECL(Class) \
public: \
	static MTypeClass* New() { return static_cast<MTypeClass*>(new Class()); } \
	static MTypeCreator<Class> s_##Class##Creator; \


#define M_TYPE_CREATE_IMPLEMENT(Class) \
	MTypeCreator<Class> Class::s_##Class##Creator = MTypeCreator<Class>();\

#define MORTY_INTERFACE(Class) \
M_TYPE_DECL(Class)

#define MORTY_INTERFACE_IMPLEMENT(Class, BaseClass) \
M_TYPE_IMPLEMENT(Class, BaseClass)


#define MORTY_CLASS(Class) \
M_TYPE_DECL(Class) \
M_TYPE_CREATE_DECL(Class) \

#define MORTY_CLASS_IMPLEMENT(Class, BaseClass)\
M_TYPE_IMPLEMENT(Class, BaseClass)\
M_TYPE_CREATE_IMPLEMENT(Class)\



class MType
{
public:
	MType(const MString& strName, const MType* pBaseType) : m_strName(strName)
		, m_pBaseType(pBaseType)
		, m_unDeep(pBaseType == nullptr ? 0 : pBaseType->m_unDeep + 1) {
	}
	~MType() {}

public:
	const MString m_strName;
	const MType* m_pBaseType;
	const int m_unDeep;
};

template <typename T>
class MTypeCreator
{
public:
	MTypeCreator<T>();
};

struct MDynamicTypeInfo
{
	MDynamicTypeInfo() :m_funcNew(nullptr), m_pType(nullptr) {}

	std::function<class MTypeClass* (void)> m_funcNew;
	const MType* m_pType;
};

class MORTY_API MTypeClass
{
public:
	MTypeClass() {};
	virtual ~MTypeClass() {}


	MString GetTypeName() {
			return GetType()->m_strName;
	}

	static const MType* GetClassType();
	static MString GetClassTypeName() { return MTypeClass::GetClassType()->m_strName; }
	virtual const MType* GetType() const { return MTypeClass::GetClassType(); };

	template<class T>
	static bool CheckNull(T* ptr)
	{
		return ptr == nullptr;
	}

	template <class T>
	T* DynamicCast() const
	{
		if (CheckNull(this)) return nullptr;
		const MType* pTypeIdent = GetType();
		const MType* pClassIdent = T::GetClassType();
		for (int i = pTypeIdent->m_unDeep - pClassIdent->m_unDeep; i > 0; --i)
			pTypeIdent = pTypeIdent->m_pBaseType;

		if (pTypeIdent == pClassIdent)
			return (T*)(this);
		return nullptr;
	}

	template<class Target, class Source>
	static std::shared_ptr<Target> DynamicCast(std::shared_ptr<Source> pointer)
	{
		if (nullptr == pointer) return nullptr;
		if (Target* ptr = (pointer.get())->template DynamicCast<Target>())
		{
			return std::shared_ptr<Target>(std::move(pointer), ptr);
		}
		return nullptr;
	}


	template<typename T1, typename T2>
	static bool IsType()
	{
		const MType* pTypeIdent = T1::GetClassType();
		const MType* pClassIdent = T2::GetClassType();
		for (int i = pTypeIdent->m_unDeep - pClassIdent->m_unDeep; i > 0; --i)
			pTypeIdent = pTypeIdent->m_pBaseType;

		return pTypeIdent == pClassIdent;
	}

	static bool IsType(const MType* a, const MType* b)
	{
		for (int i = a->m_unDeep - b->m_unDeep; i > 0; --i)
			a = a->m_pBaseType;

		return a == b;
	}

	template<typename T>
	static void RegisterTypedClass() {
		const MString& strName = T::GetClassType()->m_strName;
		MDynamicTypeInfo info;
		info.m_funcNew = &T::New;
		info.m_pType = T::GetClassType();
		GetNameTable()[strName] = info;
		GetTypeTable()[T::GetClassType()] = info;
	}

	static MTypeClass* New(const MString& strTypeName);
	static MTypeClass* New(const MType* type);
	static const MType* GetType(const MString& strTypeName);

public:

	static std::map<const MType*, MDynamicTypeInfo>& GetTypeTable();
	static std::map<MString, MDynamicTypeInfo>& GetNameTable();
};

template <typename T>
MTypeCreator<T>::MTypeCreator()
{
	MTypeClass::RegisterTypedClass<T>();
}
