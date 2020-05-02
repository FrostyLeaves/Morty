/**
 * @File         MStaticMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSTATICMESHINSTANCE_H_
#define _M_MSTATICMESHINSTANCE_H_
#include "MGlobal.h"
#include "MVertex.h"
#include "MMesh.h"
#include "MResource.h"

#include "MIModelMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MMultiLevelMesh;
class MModelResource;
class MORTY_CLASS MStaticMeshInstance : public MIModelMeshInstance
{
public:
	M_OBJECT(MStaticMeshInstance);
    MStaticMeshInstance();
    virtual ~MStaticMeshInstance();

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override;

	virtual MBoundsAABB* GetBoundsAABB() override;
	virtual MBoundsSphere* GetBoundsSphere() override;

public:

	virtual void SetMeshData(MModelMeshStruct* pMeshData);
	virtual MIMesh* GetMesh() override;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) override;

public:

	virtual void OnDelete() override;

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;

private:

	MModelMeshStruct* m_pMesh;
	MResourceKeeper m_Material;
	MBoundsAABB m_BoundsAABB;
	MBoundsSphere m_BoundsSphere;
	bool m_bBoundsAABBDirty;
	bool m_bBoundsSphereDirty;
};

#endif
