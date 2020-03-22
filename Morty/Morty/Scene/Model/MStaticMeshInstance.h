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

#include "MIModelMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MMeshDetailMap;
class MModelResource;
class MORTY_CLASS MStaticMeshInstance : public MIModelMeshInstance
{
public:
	M_OBJECT(MStaticMeshInstance);
    MStaticMeshInstance();
    virtual ~MStaticMeshInstance();

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override { return m_pMaterial; }

	virtual MBoundsAABB* GetBoundsAABB() override;

public:

	virtual void SetMeshData(MModelMeshData* pMeshData);
	virtual MIMesh* GetMesh() override;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) override;

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;

private:

	MModelMeshData* m_pMesh;
	MMaterial* m_pMaterial;
	MBoundsAABB* m_pBoundsAABB;
	bool m_bBoundsAABBDirty;
};


#endif
