/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

namespace morty
{

enum class MShaderParamType
{
    Int,
    Float,
    Float3,
    Float4,
    Color
};

enum class MShaderParamResourceType
{
    Texture2D,
    TextureCubeMap,
};

enum class MInstanceDataType
{
    Property = 0,
    TextureIndex
};

struct MShaderParamAttribute {
    MStringId        displayName;
    MStringId        name;
    MShaderParamType type;
};

struct MShaderParamResource {
    MStringId                displayName;
    MStringId                name;
    MShaderParamResourceType type;
};

struct MShaderInstancingAttribute {
    MStringId                          name;
    std::vector<MShaderParamAttribute> properties;
};

class MORTY_API MShaderPropertyBlock
{

public:
    bool                                                                      AddProperty(const MShaderParamAttribute& property);
    bool                                                                      AddResource(const MShaderParamResource& resource);

    bool                                                                      SetInstancingProperty(MInstanceDataType type, const MShaderInstancingAttribute& property);

    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamAttribute>& GetProperties() const;
    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamResource>&  GetResources() const;
    [[nodiscard]] const std::array<MShaderInstancingAttribute, 2>&            GetInstancingProperties() const;

    [[nodiscard]] MStringId                                                   GetPropertyDisplayName(const MStringId& name) const;
    [[nodiscard]] MStringId                                                   GetResourceDisplayName(const MStringId& name) const;
    [[nodiscard]] MStringId                                                   GetInstancingName(MInstanceDataType type) const;

    void                                                                      Merge(const MShaderPropertyBlock& other);
    void                                                                      Clear();

private:
    std::unordered_map<MStringId, MShaderParamAttribute> m_properties;
    std::unordered_map<MStringId, MShaderParamResource>  m_resources;
    std::array<MShaderInstancingAttribute, 2>            m_instancingProperties;
};

}// namespace morty