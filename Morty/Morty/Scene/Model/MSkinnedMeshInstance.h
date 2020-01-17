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
#include "MIMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MSkeletonInstance;
class MORTY_CLASS MSkinnedMeshInstance : public MIMeshInstance
{
public:
	M_OBJECT(MSkinnedMeshInstance);
    MSkinnedMeshInstance();
    virtual ~MSkinnedMeshInstance();

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override { return m_pMaterial; }

	virtual MBoundsAABB* GetBoundsAABB() override;

public:

	virtual void SetMesh(MIMesh* pMesh) override;
	virtual MIMesh* GetMesh() override { return m_pMesh; }

	virtual void SetSkeletonInstance(MSkeletonInstance* pSkeletonIns) { m_pSkeletonInstance = pSkeletonIns; }

 	virtual void SetDefaultOBB(MBoundsOBB* pBoundsOBB) { m_pDefaultBoundsOBB = pBoundsOBB; }
 	virtual MBoundsOBB* GetDefaultOBB() { return m_pDefaultBoundsOBB; }

protected:

	void UpdateSkeletonBoundsOBB();

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;
private:

	MIMesh* m_pMesh;
	MMaterial* m_pMaterial;
	MBoundsOBB* m_pDefaultBoundsOBB;
	MBoundsAABB* m_pBoundsAABB;
	bool m_bBoundsAABBDirty;
	MSkeletonInstance* m_pSkeletonInstance;
};


#endif
