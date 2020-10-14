/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRESOURCE_H_
#define _M_MRESOURCE_H_
#include "MGlobal.h"
#include "MString.h"
#include "MRefCounter.h"
#include "MTypedClass.h"

#include <vector>
#include <functional>

class MResourceLoader;
class MResourceManager;
class MResourceKeeper;
class MEngine;
class MObject;

#define M_I_RESOURCE(Class) \
MTypedInterfaceSign(Class)

#define M_RESOURCE(Class) \
MTypedClassSign(Class);

#define M_I_RESOURCE_IMPLEMENT(Class, BaseClass) \
MTypedInterfaceImplement(Class, BaseClass)

#define M_RESOURCE_IMPLEMENT(Class, BaseClass)\
MTypedClassImplement(Class, BaseClass)


class MORTY_CLASS MResource : public MRefCounter, public MTypedClass
{
	M_RESOURCE(MResource)
public:
	enum EResReloadType
	{
		EDefault = 0,

		EUserDef = 1000,
	};
public:
    MResource();
    virtual ~MResource();

	static MString GetSuffix(const MString& strPath);
	static MString GetFolder(const MString& strPath);
	static MString GetFileName(const MString& strPath);

	MEngine* GetEngine() const { return m_pEngine; }

	MResourceID GetResourceID() const { return m_unResourceID; }
	uint32_t GetType() const { return m_unResourceType; }

	MEngine* GetEngine() { return m_pEngine; }

	MResourceManager* GetResourceManager();

	MString GetResourcePath() { return m_strResourcePath; }

public:

	virtual void OnCreated() {}
	virtual void OnDelete() {}

	virtual void Decode(MString& strCode) {}
	virtual void Encode(MString& strCode) {}

	virtual bool Save() { return SaveTo(m_strResourcePath); }
	virtual bool SaveTo(const MString& strResourcePath) { return true; }

	virtual void CopyFrom(const MResource* pResource) {}

	void ReplaceFrom(MResource* pResource);
protected:

	virtual bool Load(const MString& strResourcePath) = 0;

	virtual void OnReferenceZero() override;
	
	void OnReload(const uint32_t& eReloadType = EResReloadType::EDefault);

protected:
    
    friend class MResourceManager;
	friend class MResourceLoader;
	friend class MResourceKeeper;

	MString m_strResourcePath;
    MResourceID m_unResourceID;
	uint32_t m_unResourceType;
	MEngine* m_pEngine;

	std::vector<MResourceKeeper*> m_vKeeper;

};

class MORTY_CLASS MResourceKeeper
{
public:
	
	typedef std::function<bool(const uint32_t& eReloadType)> MResChangedFunction;
public:
	MResourceKeeper();
	MResourceKeeper(MResource* pResource);
	MResourceKeeper(const MResourceKeeper& cHolder);
	virtual ~MResourceKeeper();

	MString GetResourcePath() { return m_pResource ? m_pResource->GetResourcePath() : ""; }

	void SetResource(MResource* pResource);
	MResource* GetResource(){ return m_pResource; }

	const MResourceKeeper& operator = (const MResourceKeeper& keeper);
	MResource* operator = (MResource* pResource);

template <class T>
	T* GetResource() { return m_pResource->DynamicCast<T>(); }

	void SetResChangedCallback(const MResChangedFunction& function)
	{
		m_funcReloadCallback = function;
	}

private:

	friend class MResource;
	MResChangedFunction m_funcReloadCallback;

	MResource* m_pResource;
};

#endif
