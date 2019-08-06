/**
 * @File         MModel
 * 
 * @Created      2019-08-06 17:49:01
 *
 * @Author       Morty
**/

#ifndef _M_MMODEL_H_
#define _M_MMODEL_H_
#include "MGlobal.h"

#include <vector>

class MMesh;
class MORTY_CLASS MModel
{
public:
    MModel();
    virtual ~MModel();

public:

private:
    
    friend class MModelLoader;
    
    std::vector<MMesh*> m_vMeshes;
    

};


#endif
