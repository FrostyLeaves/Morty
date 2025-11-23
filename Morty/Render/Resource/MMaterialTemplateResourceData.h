/**
 * @File         MMaterialResourceData
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterialTemplate.h"
#include "Material/MMaterialPass.h"

namespace morty
{

struct MORTY_API MMaterialTemplateResourceData : public MFbResourceData {
    //RawData
    MPath     shaderPath;
    MShaderMacro                                           shaderMacro;
    std::unordered_map<MStringId, std::unique_ptr<MMaterialPass>> materialPasses;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;

    void                      Deserialize(const void* pBufferPointer) override;
};


class MORTY_API MMaterialTemplateResourceDataLoader
    : public MResourceLoaderTemplate<MMaterialTemplate, MMaterialTemplateResourceData>
{
public:
    static MString              GetResourceTypeName() { return "MaterialTemplate"; }

    static std::vector<MString> GetSuffixList() { return {"mat_temp"}; }
};

}// namespace morty