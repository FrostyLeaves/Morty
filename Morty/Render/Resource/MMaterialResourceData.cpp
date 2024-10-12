#include "MMaterialResourceData.h"
#include "Flatbuffer/MMaterial_generated.h"
#include "MMaterialResource.h"

using namespace morty;

flatbuffers::Offset<void> MMaterialResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    std::vector<flatbuffers::Offset<fbs::MMaterialTexture>> fbTextureOffsets;
    for (const auto& texture: vTextures)
    {
        const auto                   fbName    = fbb.CreateString(texture.name);
        const auto                   fbTexture = fbb.CreateString(texture.value);

        fbs::MMaterialTextureBuilder builder(fbb);
        builder.add_name(fbName);
        builder.add_texture(fbTexture.o);
        fbTextureOffsets.push_back(builder.Finish());
    }

    std::vector<flatbuffers::Offset<fbs::MMaterialProperty>> fbPropertyOffsets;
    for (const auto& param: vProperty)
    {
        const auto                    fbName     = fbb.CreateString(param.name);
        const auto                    fbProperty = param.value.Serialize(fbb);

        fbs::MMaterialPropertyBuilder builder(fbb);
        builder.add_name(fbName);
        builder.add_property(fbProperty.o);
        fbPropertyOffsets.push_back(builder.Finish());
    }

    const auto            fbTextures = fbb.CreateVector(fbTextureOffsets);
    const auto            fbProperty = fbb.CreateVector(fbPropertyOffsets);
    const auto            fbTemplate = fbb.CreateString(strTemplateResource);

    fbs::MMaterialBuilder builder(fbb);

    builder.add_material_textures(fbTextures);
    builder.add_material_property(fbProperty);
    builder.add_material_template(fbTemplate);

    return builder.Finish().Union();
}

void MMaterialResourceData::Deserialize(const void* pBufferPointer)
{
    const fbs::MMaterial* fbData = fbs::GetMMaterial(pBufferPointer);

    if (fbData->material_template()) { strTemplateResource = fbData->material_template()->c_str(); }

    if (fbData->material_property())
    {
        const size_t nPropertyNum = fbData->material_property()->size();
        vProperty.resize(nPropertyNum);
        for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
        {
            const auto fbProperty = fbData->material_property()->Get(static_cast<uint32_t>(nIdx));
            vProperty[nIdx].name  = fbProperty->name()->c_str();
            vProperty[nIdx].value.Deserialize(fbProperty->property());
        }
    }

    if (fbData->material_textures())
    {
        const size_t nTextureNum = fbData->material_textures()->size();
        vTextures.resize(nTextureNum);
        for (size_t nIdx = 0; nIdx < nTextureNum; ++nIdx)
        {
            const auto fbTexture  = fbData->material_textures()->Get(static_cast<uint32_t>(nIdx));
            vTextures[nIdx].name  = fbTexture->name()->c_str();
            vTextures[nIdx].value = fbTexture->texture()->str();
        }
    }
}
