/**
 * @File         MMaterialResourceData
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterialTemplate.h"

namespace morty
{

struct MORTY_API MMaterialTemplateResourceData : public MTextResourceData {
    //RawData
    std::array<MPath, size_t(MEShaderType::TOTAL_NUM)> vShaders;

    MShaderMacro                                       shaderMacro;
    MEMaterialType                                     eMaterialType;
    MECullMode                                         eCullMode;

    void                                               Serialize(std::vector<MByte>& output) const override;
    void                                               Deserialize(const std::vector<MByte>& buffer) override;
};


class MORTY_API MMaterialTemplateResourceDataLoader
    : public MResourceLoaderTemplate<MMaterialTemplate, MMaterialTemplateResourceData>
{
public:
    static MString              GetResourceTypeName() { return "MaterialTemplate"; }

    static std::vector<MString> GetSuffixList() { return {"mat_temp"}; }
};

}// namespace morty