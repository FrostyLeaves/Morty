/**
 * @File         MAnimationMeshInstance
 * 
 * @Created      2019-12-13 20:24:34
 *
 * @Author       Pobrecito
**/

#ifndef _M_SKINNEDMESHINSTANCE_H_
#define _M_SKINNEDMESHINSTANCE_H_
#include "MGlobal.h"
#include "MVertex.h"
#include "MIModelMeshInstance.h"
#include "MResource.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MSkeletonInstance;
class MORTY_CLASS MSkinnedMeshInstance : public MIModelMeshInstance
{
public:
	M_OBJECT(MSkinnedMeshInstance);
    MSkinnedMeshInstance();
    virtual ~MSkinnedMeshInstance();

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override;

	virtual MBoundsAABB* GetBoundsAABB() override;
	virtual MBoundsSphere* GetBoundsSphere() override;
public:

	virtual void SetMeshData(MModelMeshStruct* pMeshData);
	virtual MIMesh* GetMesh() override;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) override;

	virtual void SetSkeletonInstance(MSkeletonInstance* pSkeletonIns) { m_pSkeletonInstance = pSkeletonIns; }

public:

	virtual void OnDelete() override;

protected:

	void UpdateSkeletonBoundsOBB();

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
	
	MSkeletonInstance* m_pSkeletonInstance;
};


#endif
