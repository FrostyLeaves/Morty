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

struct MShaderParamAttribute {
    MString          name;
    MShaderParamType type;
};

struct MShaderParamResource {
    MString                  name;
    MShaderParamResourceType type;
};

class MORTY_API MShaderPropertyBlock
{

public:
    bool AddProperty(const MStringId& name, const MShaderParamAttribute& property);
    bool AddResource(const MStringId& name, const MShaderParamResource& resource);
    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamAttribute>& GetProperties() const;
    [[nodiscard]] const std::unordered_map<MStringId, MShaderParamResource>&  GetResources() const;
    void                                                                      Merge(const MShaderPropertyBlock& other);
    void                                                                      Clear();

private:
    std::unordered_map<MStringId, MShaderParamAttribute> m_properties;
    std::unordered_map<MStringId, MShaderParamResource>  m_resources;
};

}// namespace morty