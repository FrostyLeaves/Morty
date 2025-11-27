#include "MMaterialResourceData.h"
#include "MMaterialResource.h"
#include "Flatbuffer/MMaterial_generated.h"

using namespace morty;


YAML::Node MMaterialResourceData::Serialize() const
{
    YAML::Node root;

    // Serialize material template
    root["MaterialTemplate"] = strTemplateResource;

    // Serialize properties
    if (!vProperty.empty())
    {
        root["Properties"] = YAML::Node(YAML::NodeType::Map);
        for (const auto& prop: vProperty) { root["Properties"][prop.name] = prop.value.SerializeYaml(); }
    }

    // Serialize textures
    if (!vTextures.empty())
    {
        root["Textures"] = YAML::Node(YAML::NodeType::Map);
        for (const auto& tex: vTextures) { root["Textures"][tex.name] = tex.value; }
    }

    return root;
}
void MMaterialResourceData::Deserialize(const YAML::Node& root)
{
    if (!root.IsMap()) return;

    // Deserialize material template
    if (root["MaterialTemplate"]) { strTemplateResource = root["MaterialTemplate"].as<std::string>(); }

    // Deserialize properties
    vProperty.clear();
    if (root["Properties"] && root["Properties"].IsMap())
    {
        for (const auto& propNode: root["Properties"])
        {
            Property prop;
            prop.name = propNode.first.as<std::string>();
            prop.value.DeserializeYaml(propNode.second);

            if (prop.value.IsValid()) { vProperty.push_back(prop); }
        }
    }

    // Deserialize textures
    vTextures.clear();
    if (root["Textures"] && root["Textures"].IsMap())
    {
        for (const auto& texNode: root["Textures"])
        {
            Texture tex;
            tex.name  = texNode.first.as<std::string>();
            tex.value = texNode.second.as<std::string>();
            vTextures.push_back(tex);
        }
    }
}


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
