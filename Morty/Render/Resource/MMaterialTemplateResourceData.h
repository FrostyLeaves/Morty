/**
 * @File         MMaterialResourceData
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterialPass.h"
#include "Material/MMaterialTemplate.h"


namespace morty
{

struct MORTY_API MMaterialTemplateResourceData : public MYamlResourceData {
    //RawData
    MPath                                                         shaderPath;
    MShaderMacro                                                  shaderMacro;
    std::unordered_map<MStringId, std::unique_ptr<MMaterialPass>> materialPasses;

    flatbuffers::Offset<void>                                     Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                                                          Deserialize(const void* pBufferPointer);

    YAML::Node                                                    Serialize() const override;
    void                                                          Deserialize(const YAML::Node& node) override;
};


class MORTY_API MMaterialTemplateResourceDataLoader
    : public MResourceLoaderTemplate<MMaterialTemplate, MMaterialTemplateResourceData>
{
public:
    static MString              GetResourceTypeName() { return "MaterialTemplate"; }

    static std::vector<MString> GetSuffixList() { return {"mat_temp"}; }
};

}// namespace morty