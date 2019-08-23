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

class MResourceLoader;

class MORTY_CLASS MResource
{
public:
    MResource();
    virtual ~MResource();

protected:

	virtual bool Load(const MString& strResourcePath) = 0;

private:
    
    friend class MResourceManager;
	friend class MResourceLoader;

    MResourceID m_unResourceID;

};


#endif
