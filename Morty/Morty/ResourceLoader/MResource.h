/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       Morty
**/

#ifndef _M_MRESOURCE_H_
#define _M_MRESOURCE_H_
#include "MGlobal.h"
#include "MString.h"

#include <vector>
#include <functional>

class MResourceLoader;
class MResourceManager;
class MResourceHolder;
class MEngine;


// #define SUB_RESOURCE( RES_NAME, RES_TYPE ) \
// private : RES_TYPE* m_pSubRes_##RES_NAME;	\
// RES_TYPE* Get##RES_NAME(){ return m_pSubRes_##RES_NAME; } \
// public : void Load##RES_NAME(MResource* pResource) { \
// 	if (RES_TYPE* pTypeResource = dynamic_cast<RES_TYPE*>(pResource)) { \
// 		m_pSubRes_##RES_NAME = pTypeResource; \
// 	} \
// } \
// 		


class MORTY_CLASS MResource
{
public:
    MResource();
    virtual ~MResource();

	static MString GetSuffix(const MString& strPath);

//	MResourceManager* GetResourceManager(){ return m_pResourceManager; }

public:

	virtual void OnCreated() {}

protected:

	virtual bool Load(const MString& strResourcePath) = 0;
	
	void OnReload();

protected:
    
    friend class MResourceManager;
	friend class MResourceLoader;
	friend class MResourceHolder;

    MResourceID m_unResourceID;
	MEngine* m_pEngine;

	std::vector<MResourceHolder*> m_vHolder;

};

class MORTY_CLASS MResourceHolder
{
public:
	typedef std::function<void()> MResChangedFunction;
public :
	MResourceHolder(MResource* pResource);
	virtual ~MResourceHolder();

	MResource* GetResource(){ return m_pResource; }

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
