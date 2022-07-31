/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRESOURCE_H_
#define _M_MRESOURCE_H_
#include "Utility/MGlobal.h"
#include "Type/MType.h"

#include <vector>
#include <functional>

class MResourceLoader;
class MResourceSystem;
class MResourceKeeper;
class MEngine;
class MObject;

enum class MEResourceType;

class MORTY_API MResource : public MTypeClass
{
	MORTY_INTERFACE(MResource)
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

	MEngine* GetEngine() { return m_pEngine; }

	MResourceSystem* GetResourceSystem();

	MString GetResourcePath() { return m_strResourcePath; }

	std::shared_ptr<MResource> GetShared();

public:

	virtual void OnCreated() {}
	virtual void OnDelete() {}

	virtual void Decode(MString& strCode) {}
	virtual void Encode(MString& strCode) {}

	virtual bool Save() { return SaveTo(m_strResourcePath); }
	virtual bool SaveTo(const MString& strResourcePath) { return true; }

	virtual void CopyFrom(std::shared_ptr<const MResource> pResource) {}

	void ReplaceFrom(std::shared_ptr<MResource> pResource);
protected:

	virtual bool Load(const MString& strResourcePath) = 0;

	void OnReload(const uint32_t& eReloadType = EResReloadType::EDefault);

protected:
    
    friend class MResourceSystem;
	friend class MResourceLoader;
	friend class MResourceKeeper;

	MString m_strResourcePath;
    MResourceID m_unResourceID;
	MEngine* m_pEngine;

	std::vector<MResourceKeeper*> m_vKeeper;

	std::weak_ptr<MResource> m_self;

};

class MORTY_API MResourceKeeper
{
public:
	
	typedef std::function<bool(const uint32_t& eReloadType)> MResChangedFunction;
public:
	MResourceKeeper();
	MResourceKeeper(std::shared_ptr<MResource> pResource);
	MResourceKeeper(const MResourceKeeper& cHolder);
	virtual ~MResourceKeeper();

	MString GetResourcePath() const { return m_pResource ? m_pResource->GetResourcePath() : ""; }

	void SetResource(std::shared_ptr<MResource> pResource);
	std::shared_ptr<MResource> GetResource(){ return m_pResource; }

	const MResourceKeeper& operator = (const MResourceKeeper& keeper);
	std::shared_ptr<MResource> operator = (std::shared_ptr<MResource> pResource);

	template <class T>
	std::shared_ptr<T> GetResource() { return m_pResource ? std::dynamic_pointer_cast<T>(m_pResource) : nullptr; }

	void SetResChangedCallback(const MResChangedFunction& function)
	{
		m_funcReloadCallback = function;
	}

private:

	friend class MResource;
	MResChangedFunction m_funcReloadCallback;

	std::shared_ptr<MResource> m_pResource;
};

#endif
