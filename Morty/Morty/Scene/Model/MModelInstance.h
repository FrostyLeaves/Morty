/**
 * @File         MModelInstance
 * 
 * @Created      2019-08-29 20:54:27
 *
 * @Author       Morty
**/

#ifndef _M_MMODELINSTANCE_H_
#define _M_MMODELINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MResource;
class MModelResource;
class MORTY_CLASS MModelInstance : public M3DNode
{
public:
	M_OBJECT(MModelInstance);
    MModelInstance();
    virtual ~MModelInstance();

public:

	bool Load(MResource* pResource);

	MModelResource* GetResource(){ return m_pResource; }

private:

	MModelResource* m_pResource;
};

#endif
