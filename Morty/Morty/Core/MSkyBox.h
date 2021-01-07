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
class MShaderParamSet;
struct MShaderConstantParam;
class MORTY_CLASS MSkyBox : public MObject
{
public:
	M_OBJECT(MSkyBox);
    MSkyBox();
    virtual ~MSkyBox();

	MTextureCube* GetTextureCube(){ return m_pTextureCube; }

	virtual bool Load(MResource* pResource);

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	MIMesh* GetMesh(){ return m_pBoxMesh; }
	MMaterial* GetMaterial() { return m_pMaterial; }

	MShaderParamSet* GetShaderMeshParamSet() { return m_pMeshParamSet; }
	MShaderConstantParam* GetShaderTransformParam() { return m_pTransformParam; }
public: 

private:
	MMesh<Vector3>* m_pBoxMesh;
	MMaterial* m_pMaterial;
	MShaderParamSet* m_pMeshParamSet;
	MShaderConstantParam* m_pTransformParam;

	MTextureCube* m_pTextureCube;
	MResourceKeeper m_TextureCubeResource;

};


#endif
