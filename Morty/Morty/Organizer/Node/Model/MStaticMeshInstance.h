/**
 * @File         MStaticMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       DoubleYe
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
class MMeshResource;
class MModelResource;
class MORTY_API MStaticMeshInstance : public MIModelMeshInstance
{
public:
	M_OBJECT(MStaticMeshInstance);
    MStaticMeshInstance();
    virtual ~MStaticMeshInstance();

public:

	void Load(MResource* pResource);

	void SetMeshResourcePath(const MString& strResourcePath);
	MString GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

	virtual MIMesh* GetMesh() override;

	virtual MSkeletonInstance* GetSkeletonInstance() override;

	virtual MBoundsAABB* GetBoundsAABB() override;
	virtual MBoundsSphere* GetBoundsSphere() override;

public:

	virtual void OnDelete() override;

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;

private:

	MMeshResource* m_pMesh;
	MResourceKeeper m_Mesh;
	MBoundsAABB m_BoundsAABB;
	MBoundsSphere m_BoundsSphere;
	bool m_bBoundsAABBDirty;
	bool m_bBoundsSphereDirty;

};

#endif
