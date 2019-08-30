/**
 * @File         MSpatial
 * 
 * @Created      2019-08-29 20:54:27
 *
 * @Author       Morty
**/

#ifndef _M_MSPATIAL_H_
#define _M_MSPATIAL_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MResource;
class MORTY_CLASS MSpatial : public M3DNode
{
public:
    MSpatial();
    virtual ~MSpatial();

public:

	bool Load(MResource* pResource);

	MResource* GetResource(){ return m_pResource; }

private:

	MResource* m_pResource;
};


#endif
