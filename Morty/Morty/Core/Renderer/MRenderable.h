/**
 * @File         MRenderable
 * 
 * @Created      2021-04-26 16:59:16
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERABLE_H_
#define _M_MRENDERABLE_H_
#include "MGlobal.h"

class MIMesh;
class MMaterial;
class MShaderParamSet;
class MORTY_API MRenderable
{
public:
	MRenderable() {}
	virtual ~MRenderable() {}

public:

	virtual MIMesh* GetMesh() = 0;

	virtual MMaterial* GetMaterial() = 0;

	virtual MShaderParamSet* GetShaderMeshParamSet() = 0;

private:

};


#endif
