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
class MResourceRef;
class MEngine;
class MObject;

enum class MEResourceType;

class MORTY_API MResource : public MTypeClass
{
	MORTY_INTERFACE(MResource)
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

	void OnReload();

protected:
    
    friend class MResourceSystem;
	friend class MResourceLoader;
	friend class MResourceRef;

	MString m_strResourcePath;
    MResourceID m_unResourceID;
	MEngine* m_pEngine;

	std::vector<MResourceRef*> m_vKeeper;

	std::weak_ptr<MResource> m_self;

};

class MORTY_API MResourceRef final
{
public:
	
	typedef std::function<bool()> MResChangedFunction;
public:
	MResourceRef();
	MResourceRef(std::shared_ptr<MResource> pResource);
	MResourceRef(const MResourceRef& cHolder);
	virtual ~MResourceRef();

	MString GetResourcePath() const { return m_pResource ? m_pResource->GetResourcePath() : ""; }

	void SetResource(std::shared_ptr<MResource> pResource);
	std::shared_ptr<MResource> GetResource() const { return m_pResource; }

	const MResourceRef& operator = (const MResourceRef& keeper);
	std::shared_ptr<MResource> operator = (std::shared_ptr<MResource> pResource);

	template <class T>
	std::shared_ptr<T> GetResource() const { return m_pResource ? std::dynamic_pointer_cast<T>(m_pResource) : nullptr; }

	void SetResChangedCallback(const MResChangedFunction& function)
	{
		m_funcReloadCallback = function;
	}

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(MEngine* pEngine, const void* pBufferPointer);

private:

	friend class MResource;
	MResChangedFunction m_funcReloadCallback;

	std::shared_ptr<MResource> m_pResource;
};

#endif
