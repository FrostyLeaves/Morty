/**
 * @File         M3DMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       Morty
**/

#ifndef _M_M3DMESHINSTANCE_H_
#define _M_M3DMESHINSTANCE_H_
#include "MGlobal.h"

#include "M3DNode.h"

class MORTY_CLASS M3DMeshInstance : public M3DNode
{
public:
    M3DMeshInstance();
    virtual ~M3DMeshInstance();

public:

	void Load(const char* svPath);

private:

};


#endif
