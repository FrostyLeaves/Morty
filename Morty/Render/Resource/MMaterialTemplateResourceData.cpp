#include "MMaterialTemplateResourceData.h"
#include "MMaterialResource.h"
#include "Material/MMaterialPass.h"
#include "yaml-cpp/yaml.h"
#include "Flatbuffer/MMaterialTemplate_generated.h"
#include "Flatbuffer/MMaterial_generated.h"


using namespace morty;


YAML::Node MMaterialTemplateResourceData::Serialize() const
{
    YAML::Node root;

    // Serialize shader path
    root["ShaderPath"] = shaderPath;

    // Serialize shader macro
    root["ShaderMacro"] = shaderMacro.SerializeYaml();

    // Serialize material passes as array (PassName is already stored in PassData)
    root["MaterialPasses"] = YAML::Node(YAML::NodeType::Sequence);
    for (const auto& [passName, pass]: materialPasses)
    {
        if (pass) { root["MaterialPasses"].push_back(pass->SerializeYaml()); }
    }

    return root;
}

void MMaterialTemplateResourceData::Deserialize(const YAML::Node& root)
{
    if (!root.IsMap()) return;

    // Deserialize shader path
    if (root["ShaderPath"]) { shaderPath = root["ShaderPath"].as<std::string>(); }

    // Deserialize shader macro
    if (root["ShaderMacro"]) { shaderMacro.DeserializeYaml(root["ShaderMacro"]); }

    // Deserialize material passes
    materialPasses.clear();
    if (root["MaterialPasses"] && root["MaterialPasses"].IsSequence())
    {
        for (const auto& passNode: root["MaterialPasses"])
        {
            // Check if it's the new format (pass data directly) or old format (with PassName and PassData)
            if (passNode["PassData"])
            {
                // Old format: { PassName: "...", PassData: {...} }
                if (passNode["PassName"])
                {
                    MStringId passName(passNode["PassName"].as<std::string>());
                    auto      pass = std::make_unique<MMaterialPass>(nullptr);
                    pass->DeserializeYaml(passNode["PassData"]);
                    materialPasses[passName] = std::move(pass);
                }
            }
            else if (passNode["PassName"])
            {
                // New format: pass data directly with PassName inside
                auto pass = std::make_unique<MMaterialPass>(nullptr);
                pass->DeserializeYaml(passNode);
                MStringId passName = pass->GetPassName();
                materialPasses[passName] = std::move(pass);
            }
        }
    }
}

flatbuffers::Offset<void> MMaterialTemplateResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    const auto                                           fbShaderPath = fbb.CreateString(shaderPath);
    const auto                                           fbMacro      = shaderMacro.Serialize(fbb);

    // Serialize material passes
    std::vector<flatbuffers::Offset<fbs::MMaterialPass>> materialPassOffsets;
    for (const auto& [passName, pass]: materialPasses)
    {
        if (pass) { materialPassOffsets.push_back(pass->Serialize(fbb).o); }
    }
    auto                          materialPassVector = fbb.CreateVector(materialPassOffsets);

    fbs::MMaterialTemplateBuilder builder(fbb);

    builder.add_shader_resource(fbShaderPath.o);
    builder.add_material_macro(fbMacro.o);
    builder.add_material_pass(materialPassVector);

    return builder.Finish().Union();
}

void MMaterialTemplateResourceData::Deserialize(const void* pBufferPointer)
{
    const fbs::MMaterialTemplate* fbData = fbs::GetMMaterialTemplate(pBufferPointer);

    shaderMacro.Deserialize(fbData->material_macro());

    if (fbData->shader_resource()) { shaderPath = fbData->shader_resource()->str(); }

    // Deserialize material passes
    materialPasses.clear();
    if (fbData->material_pass())
    {
        const auto* passVector = fbData->material_pass();
        for (uint32_t i = 0; i < passVector->size(); ++i)
        {
            const auto* fbPass = passVector->Get(i);
            if (fbPass && fbPass->pass_name())
            {
                // Create a temporary pass with nullptr template - will be corrected during loading
                auto pass = std::make_unique<MMaterialPass>(nullptr);
                pass->Deserialize(fbPass);

                MStringId passName(fbPass->pass_name()->c_str());
                materialPasses[passName] = std::move(pass);
            }
        }
    }
}
