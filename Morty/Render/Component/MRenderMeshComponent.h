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
#include "Resource/MMeshResource.h"
#include "Resource/MResource.h"
#include "Variant/MVariant.h"

namespace morty
{

class MMaterialResource;
class MBoundsAABB;
class MBoundsSphere;
class MModelInstance;
class MModelComponent;
class MShaderParameterSet;
class MSkeletonInstance;
struct MShaderUniformParam;

MORTY_ENUM MEShadowType{
        ENone            = 0,
        EOnlyDirectional = 1,
        EAllLights       = 2,
};

class MORTY_API MRenderMeshComponent : public MComponent
{
public:
    MORTY_CLASS(MRenderMeshComponent)

public:
    MRenderMeshComponent();
    ~MRenderMeshComponent() override = default;

public:
    void                                             Release() override;


    void                                             SetMaterial(const std::shared_ptr<MMaterialResource>& material);
    [[nodiscard]] std::shared_ptr<MMaterialResource> GetMaterial() const;

    void                                             SetMesh(const std::shared_ptr<MMeshResource>& mesh);
    [[nodiscard]] std::shared_ptr<MMeshResource>     GetMesh() const;

    void                                             SetInstancingData(const MVariant& value);
    MVariant                                         GetInstancingData() const;


public:
    MIMesh*      GetDrawMesh();

    void         SetShadowType(const MEShadowType& eType) { m_shadowType = eType; }

    MEShadowType GetShadowType() { return m_shadowType; }

    void         SetGenerateDirLightShadow(const bool& bGenerate);

    bool         GetGenerateDirLightShadow() const { return m_generateDirLightShadow; }

    void         SetSceneCullEnable(bool bEnable);

    bool         GetSceneCullEnable() const { return m_sceneCullEnable; }

    MComponentID GetAttachedModelComponentID() const { return m_modelComponent; }

    void         SetAttachedModelComponentID(MComponentID idx);

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(const void* pBufferPointer) override;

protected:
    PROPERTY_RESOURCE(MMeshResource) MResourceRef m_mesh;
    PROPERTY_RESOURCE(MMaterialResource) MResourceRef m_material;
    PROPERTY_ENUM MEShadowType m_shadowType;
    PROPERTY_STRUCT MVariant   m_instancingData;

    MComponentID               m_modelComponent;

    bool                       m_sceneCullEnable        = true;
    PROPERTY_VARIANT bool      m_generateDirLightShadow = true;
};

}// namespace morty