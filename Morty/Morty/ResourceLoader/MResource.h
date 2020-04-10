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

#include <vector>
#include <functional>

class MResourceLoader;
class MResourceManager;
class MResourceHolder;
class MEngine;

class MORTY_CLASS MResource : public MRefCounter
{
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

	MEngine* GetEngine() { return m_pEngine; }

	MResourceManager* GetResourceManager();

	MString GetResourcePath() { return m_strResourcePath; }

public:

	virtual void OnCreated() {}
	virtual void OnRelease() {}

	virtual void Decode(MString& strCode) {}
	virtual void Encode(MString& strCode) {}

	virtual bool Save() { return true; }
	virtual bool SaveTo(const MString& strResourcePath);

protected:

	virtual bool Load(const MString& strResourcePath) = 0;

	virtual void OnReferenceZero() override;
	
	void OnReload(const unsigned int& eReloadType = EResReloadType::EDefault);

protected:
    
    friend class MResourceManager;
	friend class MResourceLoader;
	friend class MResourceHolder;

	MString m_strResourcePath;
    MResourceID m_unResourceID;
	MEngine* m_pEngine;

	std::vector<MResourceHolder*> m_vHolder;

};

class MORTY_CLASS MResourceHolder
{
public:
	
	typedef std::function<bool(const unsigned int& eReloadType)> MResChangedFunction;
public :
	MResourceHolder(MResource* pResource);
	MResourceHolder(const MResourceHolder& cHolder);
	virtual ~MResourceHolder();

	MResource* GetResource(){ return m_pResource; }

template <class T>
	T* GetResource() { return dynamic_cast<T*>(m_pResource); }

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
