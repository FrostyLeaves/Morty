/**
 * @File         MIMeshInstance
 * 
 * @Created      2019-12-13 20:24:19
 *
 * @Author       Morty
**/

#ifndef _M_MIMESHINSTANCE_H_
#define _M_MIMESHINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MIMesh;
class MMaterial;
class MORTY_CLASS MIMeshInstance : public M3DNode
{
public:
    MIMeshInstance();
    virtual ~MIMeshInstance();

public:
	virtual void SetMesh(MIMesh* pMesh) = 0;
	virtual MIMesh* GetMesh() = 0;

	virtual void SetMaterial(MMaterial* pMaterial) = 0;
	virtual MMaterial* GetMaterial() = 0;
private:

};

#endif
