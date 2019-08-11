/**
 * @File         MResourceLoader
 * 
 * @Created      2019-08-06 17:59:45
 *
 * @Author       Morty
**/

#ifndef _M_MRESOURCELOADER_H_
#define _M_MRESOURCELOADER_H_
#include "MGlobal.h"
#include "MString.h"

class MResource;
class MORTY_CLASS MResourceLoader
{
public:
    MResourceLoader();
    virtual ~MResourceLoader();

public:

	virtual MResource* Load(const MString& svPath) = 0;
	
private:

};


#endif
