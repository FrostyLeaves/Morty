/**
 * @File         MMaterialResourceData
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MMaterialResource.h"

namespace morty
{

struct MORTY_API MMaterialResourceData : public MYamlResourceData {
    struct Property {
        MString  name;
        MVariant value;
    };

    struct Texture {
        MString name;
        MPath   value;
    };

    std::vector<Property>     vProperty;
    std::vector<Texture>      vTextures;

    MString                   strTemplateResource;

    YAML::Node                Serialize() const override;
    void                      Deserialize(const YAML::Node& node) override;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                      Deserialize(const void* pBufferPointer);
};


class MORTY_API MMaterialResourceLoader : public MResourceLoaderTemplate<MMaterialResource, MMaterialResourceData>
{
public:
    static MString              GetResourceTypeName() { return "Material"; }

    static std::vector<MString> GetSuffixList() { return {"mat"}; }
};

}// namespace morty