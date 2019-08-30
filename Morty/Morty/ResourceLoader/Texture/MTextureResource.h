/**
 * @File         MTextureResource
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       Morty
**/

#ifndef _M_MTEXTURERESOURCE_H_
#define _M_MTEXTURERESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MORTY_CLASS MTextureResource : public MResource
{
public:
    MTextureResource();
    virtual ~MTextureResource();

protected:

	virtual bool Load(const MString& strResourcePath) override;
private:

};


#endif
