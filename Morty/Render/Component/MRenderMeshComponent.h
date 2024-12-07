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

#include "Material/MMaterialPropertyProxy.h"
#include "Math/MMath.h"
#include "Resource/MMeshResource.h"
#include "Resource/MResource.h"

namespace morty
{

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

    ~MRenderMeshComponent() override;

public:
    enum class MEShadowType
    {
        ENone            = 0,
        EOnlyDirectional = 1,
        EAllLights       = 2,
    };

public:
    void                                             Release() override;

    void                                             SetMaterial(std::shared_ptr<MMaterialResource> pMaterial);

    [[nodiscard]] std::shared_ptr<MMaterialResource> GetMaterialResource() const;

    std::shared_ptr<MMaterial>                       GetMaterial();

    bool                                             SetMaterialPath(const MString& strPath);

    void                                             Load(std::shared_ptr<MResource> pResource);

    void                                             SetMeshResourcePath(const MString& strResourcePath);

    MString                                          GetMeshResourcePath() { return m_Mesh.GetResourcePath(); }

    [[nodiscard]] MResourceRef                       GetMeshResource() const { return m_Mesh; }

public:
    MIMesh*                    GetMesh();

    void                       SetShadowType(const MEShadowType& eType) { m_shadowType = eType; }

    MEShadowType               GetShadowType() { return m_shadowType; }

    void                       SetDetailLevel(const uint32_t& unLevel) { m_unDetailLevel = unLevel; }

    [[nodiscard]] uint32_t     GetDetailLevel() const { return m_unDetailLevel; }

    void                       SetGenerateDirLightShadow(const bool& bGenerate);

    [[nodiscard]] bool         GetGenerateDirLightShadow() const { return m_generateDirLightShadow; }

    void                       SetSceneCullEnable(bool bEnable);

    [[nodiscard]] bool         GetSceneCullEnable() const { return m_sceneCullEnable; }


    [[nodiscard]] MComponentID GetAttachedModelComponentID() const { return m_modelComponent; }

    void                       SetAttachedModelComponentID(MComponentID idx);

public:
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    void                      Deserialize(const void* pBufferPointer) override;

public:
    MMaterialPropertyProxy m_materialProxy;

protected:
    MResourceRef m_Mesh;
    MResourceRef m_Material;
    MEShadowType m_shadowType;
    uint32_t     m_unDetailLevel;


    MComponentID m_modelComponent;

    bool         m_sceneCullEnable        = true;
    bool         m_generateDirLightShadow = true;
};

}// namespace morty