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
class MMeshResource;
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

	void Load(MResource* pResource);

	void SetMeshResourcePath(const MString& strResourcePath);
	MString GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

	virtual MIMesh* GetMesh() override { return GetMesh(GetDetailLevel()); }
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) override;

	virtual MSkeletonInstance* GetSkeletonInstance() override;

public:

	virtual void OnDelete() override;

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

protected:

	void SetMeshData(const MString& strModelResourcePath, const int& nIndex);

protected:

	void UpdateSkeletonBoundsOBB();

protected:
	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;
private:

	MMeshResource* m_pMesh;
	MResourceKeeper m_Mesh;
	MResourceKeeper m_Material;
	MBoundsAABB m_BoundsAABB;
	MBoundsSphere m_BoundsSphere;
	bool m_bBoundsAABBDirty;
	bool m_bBoundsSphereDirty;
};


#endif
