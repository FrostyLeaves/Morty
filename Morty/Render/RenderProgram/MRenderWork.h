/**
 * @File         MRenderWork
 * 
 * @Created      2021-08-16 11:50:15
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERWORK_H_
#define _M_MRENDERWORK_H_
#include "Utility/MGlobal.h"

#include "Object/MObject.h"

class MBuffer;
class MMaterial;
class MShaderPropertyBlock;

class MORTY_API MRenderWork : public MObject
{
    MORTY_CLASS(MRenderWork)
public:
    MRenderWork();
    virtual ~MRenderWork();

};


#endif
