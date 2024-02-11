/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Type/MType.h"

MORTY_SPACE_BEGIN

class MResourceLoader;
class MResourceSystem;
class MResourceRef;
class MEngine;
class MObject;

enum class MEResourceType;

class MORTY_API MResourceData
{
public:
	MResourceData() = default;
	virtual ~MResourceData() = default;

	virtual void LoadBuffer(const std::vector<MByte>& buffer) = 0;
	virtual std::vector<MByte> SaveBuffer() const = 0;
};

class MORTY_API MFbResourceData : public MResourceData
{
public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const = 0;
	virtual void Deserialize(const void* pBufferPointer) = 0;

	void LoadBuffer(const std::vector<MByte>& buffer) override;
	std::vector<MByte> SaveBuffer() const override;
};

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
	MResourceSystem* GetResourceSystem();
	MResourceID GetResourceID() const { return m_unResourceID; }
	const MString& GetResourcePath() const { return m_strResourcePath; }

	std::shared_ptr<MResource> GetShared() const;

#if MORTY_DEBUG
	const char* GetDebugName() const;
#endif

public:

	virtual void OnCreated() {}
	virtual void OnDelete() {}

	virtual bool Load(std::unique_ptr<MResourceData>&& pResourceData) { 
		MORTY_UNUSED(pResourceData);
		return false; 
	};

	virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) {
		MORTY_UNUSED(pResourceData);
		return false;
		}

	void ReplaceFrom(std::shared_ptr<MResource> pResource);

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
	bool operator == (const MResourceRef& other) const { return GetResource() == other.GetResource(); }

	template <class T>
	std::shared_ptr<T> GetResource() const { return m_pResource ? std::dynamic_pointer_cast<T>(m_pResource) : nullptr; }

	void SetResChangedCallback(const MResChangedFunction& function)
	{
		m_funcReloadCallback = function;
	}

public:

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(MResourceSystem* pResourceSystem, const void* pBufferPointer);

private:

	friend class MResource;
	MResChangedFunction m_funcReloadCallback;

	std::shared_ptr<MResource> m_pResource;
};

MORTY_SPACE_END