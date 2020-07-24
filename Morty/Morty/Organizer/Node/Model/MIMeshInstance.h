/**
 * @File         MIMeshInstance
 * 
 * @Created      2019-12-13 20:24:19
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIMESHINSTANCE_H_
#define _M_MIMESHINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"
#include "MBounds.h"

class MIMesh;
class MMaterial;
class MModelInstance;
class MShaderParamSet;
class MModelMeshStruct;
class MSkeletonInstance;
class MShaderConstantParam;
class MORTY_CLASS MIMeshInstance : public M3DNode
{
public:
	enum MERenderOrderType
	{
		EAutoOrder = 0,
		EOrderByTransparent = 1,
		EOrderByMaterial = 2,
	};

public:
	M_OBJECT(MIMeshInstance);
    MIMeshInstance();
    virtual ~MIMeshInstance();

public:

	void SetMaterial(MMaterial* pMaterial);
	MMaterial* GetMaterial();


	MShaderParamSet* GetShaderMeshParamSet();
	void UpdateShaderMeshParam();

	bool SetMaterialPath(const MString& strPath);
	MString GetMaterialPath();

public:

	virtual MIMesh* GetMesh() = 0;
	virtual MIMesh* GetMesh(const uint32_t& unDetailLevel) = 0;

	virtual MBoundsAABB* GetBoundsAABB() = 0;
	virtual MBoundsSphere* GetBoundsSphere() = 0;

	virtual void SetGenerateDirLightShadow(const bool& bGenerate) = 0;
	virtual bool GetGenerateDirLightShadow() const = 0;

	virtual MSkeletonInstance* GetSkeletonInstance() { return nullptr; }

protected:

	virtual void WorldTransformDirty() override;
	virtual void LocalTransformDirty() override;

protected:

	void BindShaderParam(MMaterial* pMaterial);

protected:

	MResourceKeeper m_Material;
	bool m_bTransformParamDirty;

	MShaderParamSet* m_pShaderParamSet;
	MShaderConstantParam* m_pTransformParam;

	Matrix4* m_pWorldMatrixParam;
	Matrix3* m_pNormalMatrixParam;
};

#endif
