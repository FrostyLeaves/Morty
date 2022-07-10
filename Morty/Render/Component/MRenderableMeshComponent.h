/**
 * @File         MRenderableMeshComponent
 * 
 * @Created      2021-04-26 16:51:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERABLE_MESH_COMPONENT_H_
#define _M_MRENDERABLE_MESH_COMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "MMath.h"
#include "MResource.h"
#include "MMeshResource.h"

class MBoundsAABB;
class MBoundsSphere;
class MModelInstance;
class MModelComponent;
class MShaderParamSet;
class MSkeletonInstance;
struct MShaderConstantParam;
class MORTY_API MRenderableMeshComponent : public MComponent
{
public:
    MORTY_CLASS(MRenderableMeshComponent)

public:
    MRenderableMeshComponent();
    virtual ~MRenderableMeshComponent();

public:
	enum class MEShadowType
	{
		ENone = 0,
		EOnlyDirectional = 1,
		EAllLights = 2,
	};
public:

	virtual void Initialize() override;
	virtual void Release() override;

	void SetMaterial(MMaterial* pMaterial);
	MMaterial* GetMaterial();

	MShaderParamSet* GetShaderMeshParamSet();
	void UpdateShaderMeshParam();

	bool SetMaterialPath(const MString& strPath);
	MString GetMaterialPath();

	void Load(MResource* pResource);

	void SetMeshResourcePath(const MString& strResourcePath);
	MString GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

public:

	MIMesh* GetMesh();

	MBoundsAABB* GetBoundsAABB();
	MBoundsSphere* GetBoundsSphere();
	
	MSkeletonInstance* GetSkeletonInstance();

	void SetShadowType(const MEShadowType& eType) { m_eShadowType = eType; }
	MEShadowType GetShadowType() { return m_eShadowType; }

	void SetDetailLevel(const uint32_t& unLevel) { m_unDetailLevel = unLevel; }
	uint32_t GetDetailLevel() { return m_unDetailLevel; }

	void SetDrawBoundingSphere(const bool& bDrawable) { m_bDrawBoundingSphere = bDrawable; }
	bool GetDrawBoundingSphere() { return m_bDrawBoundingSphere; }

	void SetGenerateDirLightShadow(const bool& bGenerate) { m_bGenerateDirLightShadow = bGenerate; }
	bool GetGenerateDirLightShadow() const { return m_bGenerateDirLightShadow; }

	MModelComponent* GetAttachedModelComponent();

public:

//Notify

	void OnTransformDirty();
	void OnParentChanged();

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;

protected:

	void BindShaderParam(MMaterial* pMaterial);

protected:

	MResourceKeeper m_Mesh;
	MResourceKeeper m_Material;

	MShaderParamSet* m_pShaderParamSet;
	MShaderConstantParam* m_pTransformParam;

	Matrix4* m_pWorldMatrixParam;
	Matrix3* m_pNormalMatrixParam;

	MModelComponent* m_pModelComponent;

	MBoundsAABB m_BoundsAABB;
	MBoundsSphere m_BoundsSphere;

	MEShadowType m_eShadowType;
	uint32_t m_unDetailLevel;


	bool m_bDrawBoundingSphere;
	bool m_bGenerateDirLightShadow;
	bool m_bModelInstanceFound;
	bool m_bTransformParamDirty;
	bool m_bBoundsAABBDirty;
	bool m_bBoundsSphereDirty;
};


#endif
