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
	virtual MMaterial* GetMaterial() override { return m_pMaterial; }

	virtual MBoundsAABB* GetBoundsAABB() override;

public:

	virtual void SetMeshData(MModelMeshData* pMeshData);
	virtual MIMesh* GetMesh() override;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) override;

	virtual void SetSkeletonInstance(MSkeletonInstance* pSkeletonIns) { m_pSkeletonInstance = pSkeletonIns; }

protected:

	void UpdateSkeletonBoundsOBB();

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;
private:

	MModelMeshData* m_pMesh;
	MMaterial* m_pMaterial;
	MBoundsAABB* m_pBoundsAABB;
	bool m_bBoundsAABBDirty;
	MSkeletonInstance* m_pSkeletonInstance;
};


#endif
