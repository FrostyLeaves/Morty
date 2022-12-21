/**
 * @File         MRenderableMeshComponent
 * 
 * @Created      2021-04-26 16:51:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERABLE_MESH_COMPONENT_H_
#define _M_MRENDERABLE_MESH_COMPONENT_H_
#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Math/MMath.h"
#include "Resource/MResource.h"
#include "Resource/MMeshResource.h"

class MBoundsAABB;
class MBoundsSphere;
class MModelInstance;
class MModelComponent;
class MShaderPropertyBlock;
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

	void SetMaterial(std::shared_ptr<MMaterial> pMaterial);
	std::shared_ptr<MMaterial> GetMaterial();

	std::shared_ptr<MShaderPropertyBlock> GetShaderMeshParamSet();
	void UpdateShaderMeshParam();

	bool SetMaterialPath(const MString& strPath);
	MString GetMaterialPath();

	void Load(std::shared_ptr<MResource> pResource);

	void SetMeshResourcePath(const MString& strResourcePath);
	MString GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

public:

	MIMesh* GetMesh();

	MBoundsAABB* GetBoundsAABB();
	MBoundsSphere* GetBoundsSphere();
	
	std::shared_ptr<MSkeletonInstance> GetSkeletonInstance();

	void SetShadowType(const MEShadowType& eType) { m_eShadowType = eType; }
	MEShadowType GetShadowType() { return m_eShadowType; }

	void SetDetailLevel(const uint32_t& unLevel) { m_unDetailLevel = unLevel; }
	uint32_t GetDetailLevel() { return m_unDetailLevel; }

	void SetDrawBoundingSphere(const bool& bDrawable) { m_bDrawBoundingSphere = bDrawable; }
	bool GetDrawBoundingSphere() { return m_bDrawBoundingSphere; }

	void SetGenerateDirLightShadow(const bool& bGenerate) { m_bGenerateDirLightShadow = bGenerate; }
	bool GetGenerateDirLightShadow() const { return m_bGenerateDirLightShadow; }

	void SetBatchInstanceEnable(const bool& bBatch);
	bool GetBatchInstanceEnable() const { return m_bBatchInstanceEnable; }

	MModelComponent* GetAttachedModelComponent();

public:

//Notify

	void OnTransformDirty();
	void OnParentChanged();

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;

protected:

	void BindShaderParam(std::shared_ptr<MMaterial> pMaterial);

protected:

	MResourceKeeper m_Mesh;
	MResourceKeeper m_Material;

	std::shared_ptr<MShaderPropertyBlock> m_pShaderParamSet = nullptr;
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
	bool m_bBatchInstanceEnable;
	bool m_bModelInstanceFound;
	bool m_bTransformParamDirty;
	bool m_bBoundsAABBDirty;
	bool m_bBoundsSphereDirty;
};


#endif
