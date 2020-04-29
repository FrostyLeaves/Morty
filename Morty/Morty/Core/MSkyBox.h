/**
 * @File         MSkyBox
 * 
 * @Created      2019-09-22 01:39:38
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSKYBOX_H_
#define _M_MSKYBOX_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MMesh.h"
#include "MResource.h"

class MModelMeshStruct;
class MTextureCubeResource;
class MTextureCube;
class MIMeshInstance;
class MORTY_CLASS MSkyBox : public MObject
{
public:
	M_OBJECT(MSkyBox);
    MSkyBox();
    virtual ~MSkyBox();

	MTextureCube* GetTextureCube(){ return m_pTextureCube; }

	virtual bool Load(MResource* pResource);

	virtual void OnCreated() override;

	MIMeshInstance* GetMeshInstance(){ return m_pMeshInstance; }

public:

private:
	MMesh<Vector3>* m_pBoxMesh;
	MModelMeshStruct* m_pMeshData;
	MIMeshInstance* m_pMeshInstance;

	MTextureCube* m_pTextureCube;
	MResourceKeeper m_TextureCubeResource;

};


#endif
