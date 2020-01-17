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

#include "MIMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MORTY_CLASS MStaticMeshInstance : public MIMeshInstance
{
public:
	M_OBJECT(MStaticMeshInstance);
    MStaticMeshInstance();
    virtual ~MStaticMeshInstance();

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override { return m_pMaterial; }

	virtual MBoundsAABB* GetBoundsAABB() override;

public:

	virtual void SetMesh(MIMesh* pMesh) override;
	virtual MIMesh* GetMesh() override { return m_pMesh; }

	virtual void SetDefaultOBB(MBoundsOBB* pBoundsOBB) { m_pDefaultBoundsOBB = pBoundsOBB; }
	virtual MBoundsOBB* GetDefaultOBB() { return m_pDefaultBoundsOBB; }

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;

private:

	MMesh<MVertex>* m_pMesh;
	MMaterial* m_pMaterial;
	MBoundsOBB* m_pDefaultBoundsOBB;
	MBoundsAABB* m_pBoundsAABB;
	bool m_bBoundsAABBDirty;
};


#endif
