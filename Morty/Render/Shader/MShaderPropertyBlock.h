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
    TextureIndex,
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

class MORTY_API MShaderPropertyBlock
{

public:
    bool AddProperty(const MShaderParamAttribute& property);
    bool AddResource(const MShaderParamResource& resource);
    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamAttribute>& GetProperties() const;
    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamResource>&  GetResources() const;

    MStringId GetPropertyDisplayName(const MStringId& name) const;
    MStringId GetResourceDisplayName(const MStringId& name) const;

    void      Merge(const MShaderPropertyBlock& other);
    void      Clear();

    void      SetInstancingName(const MStringId& name, MInstanceDataType type)
    {
        m_instancingName[static_cast<size_t>(type)] = name;
    }
    MStringId GetInstancingName(MInstanceDataType type) const { return m_instancingName[static_cast<size_t>(type)]; }

private:
    std::unordered_map<MStringId, MShaderParamAttribute> m_properties;
    std::unordered_map<MStringId, MShaderParamResource>  m_resources;
    std::array<MStringId, 2>                             m_instancingName;
};

}// namespace morty