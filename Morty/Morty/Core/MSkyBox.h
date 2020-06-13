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

class MMaterial;
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

	MIMesh* GetMesh(){ return m_pBoxMesh; }
	MMaterial* GetMaterial() { return m_pMaterial; }

public: 

private:
	MMesh<Vector3>* m_pBoxMesh;
	MModelMeshStruct* m_pMeshData;
	MMaterial* m_pMaterial;

	MTextureCube* m_pTextureCube;
	MResourceKeeper m_TextureCubeResource;

};


#endif
