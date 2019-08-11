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

protected:

	void Clean();

private:
    
    friend class MModelResource;
    
    std::vector<MMesh*> m_vMeshes;
    

};


#endif
