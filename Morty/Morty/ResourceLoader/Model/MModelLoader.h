/**
 * @File         MModelLoader
 * 
 * @Created      2019-08-06 16:51:51
 *
 * @Author       Morty
**/

#ifndef _M_MMODELLOADER_H_
#define _M_MMODELLOADER_H_
#include "MGlobal.h"
#include "MString.h"

#include "MResourceLoader.h"


class MORTY_CLASS MModelLoader : public MResourceLoader
{
public:
    MModelLoader();
    virtual ~MModelLoader();

public:
    
	virtual MResource* Load(const MString& svPath);


private:

};


#endif
