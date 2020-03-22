/**
 * @File         MIMeshInstance
 * 
 * @Created      2019-12-13 20:24:19
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIMESHINSTANCE_H_
#define _M_MIMESHINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MIMesh;
class MMaterial;
class MBoundsOBB;
class MBoundsAABB;
class MModelMeshData;
class MModelInstance;
class MORTY_CLASS MIMeshInstance : public M3DNode
{

public:
	M_OBJECT(MIMeshInstance);
    MIMeshInstance();
    virtual ~MIMeshInstance();

public:
	virtual void SetMaterial(MMaterial* pMaterial) = 0;
	virtual MMaterial* GetMaterial() = 0;

	virtual MBoundsAABB* GetBoundsAABB() = 0;

public:
	virtual MIMesh* GetMesh() = 0;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) = 0;

private:
};

#endif
