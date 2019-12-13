/**
 * @File         MSkyBox
 * 
 * @Created      2019-09-22 01:39:38
 *
 * @Author       Morty
**/

#ifndef _M_MSKYBOX_H_
#define _M_MSKYBOX_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MMesh.h"

class MResource;
class MResourceHolder;
class MTextureCubeResource;
class MTextureCube;
class MStaticMeshInstance;
class MORTY_CLASS MSkyBox : public MObject
{
public:
	M_OBJECT(MSkyBox);
    MSkyBox();
    virtual ~MSkyBox();

	MTextureCube* GetTextureCube(){ return m_pTextureCube; }

	virtual bool Load(MResource* pResource);

	virtual void OnCreated() override;

	MStaticMeshInstance* GetMeshInstance(){ return m_pMeshInstance; }

public:

private:
	MMesh<Vector3>* m_pBoxMesh;
	MStaticMeshInstance* m_pMeshInstance;

	MTextureCube* m_pTextureCube;
	MResourceHolder* m_pTextureCubeResource;

};


#endif
