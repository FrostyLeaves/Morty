/**
 * @File         MRenderMeshComponent
 * 
 * @Created      2021-04-26 16:51:46
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Math/MMath.h"
#include "Resource/MResource.h"
#include "Resource/MMeshResource.h"

MORTY_SPACE_BEGIN

class MMaterialResource;
class MBoundsAABB;
class MBoundsSphere;
class MModelInstance;
class MModelComponent;
class MShaderPropertyBlock;
class MSkeletonInstance;
struct MShaderConstantParam;
class MORTY_API MRenderMeshComponent : public MComponent
{
public:
    MORTY_CLASS(MRenderMeshComponent)

public:
    MRenderMeshComponent();
    virtual ~MRenderMeshComponent();

public:
	enum class MEShadowType
	{
		ENone = 0,
		EOnlyDirectional = 1,
		EAllLights = 2,
	};
public:

	virtual void Release() override;

	void SetMaterial(std::shared_ptr<MMaterialResource> pMaterial);
	std::shared_ptr<MMaterialResource> GetMaterialResource() const;
	std::shared_ptr<MMaterial> GetMaterial();

	bool SetMaterialPath(const MString& strPath);

	void Load(std::shared_ptr<MResource> pResource);

	void SetMeshResourcePath(const MString& strResourcePath);
	MString GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

	MResourceRef GetMeshResource() const { return m_Mesh; }

public:

	MIMesh* GetMesh();

	void SetShadowType(const MEShadowType& eType) { m_eShadowType = eType; }
	MEShadowType GetShadowType() { return m_eShadowType; }

	void SetDetailLevel(const uint32_t& unLevel) { m_unDetailLevel = unLevel; }
	uint32_t GetDetailLevel() { return m_unDetailLevel; }

	void SetGenerateDirLightShadow(const bool& bGenerate);
	bool GetGenerateDirLightShadow() const { return m_bGenerateDirLightShadow; }

	void SetSceneCullEnable(bool bEnable);
	bool GetSceneCullEnable() const { return m_bSceneCullEnable; }


	MComponentID GetAttachedModelComponentID() const { return m_modelComponent; }
	void SetAttachedModelComponentID(MComponentID idx);

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;

protected:

	MResourceRef m_Mesh;
	MResourceRef m_Material;
	MEShadowType m_eShadowType;
	uint32_t m_unDetailLevel;

	MComponentID m_modelComponent;

	bool m_bSceneCullEnable = true;
	bool m_bGenerateDirLightShadow = true;
};

MORTY_SPACE_END