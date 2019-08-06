/**
 * @File         MResourceModel
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       Morty
**/

#ifndef _M_MODELMRESOURCE_H_
#define _M_MODELMRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"


class MModel;
class MORTY_CLASS MModelResource : public MResource
{
public:
    MModelResource();
    virtual ~MModelResource();

public:

private:
    
    friend class MModelLoader;
    
    MModel* m_pModelTemplate;

};


#endif
