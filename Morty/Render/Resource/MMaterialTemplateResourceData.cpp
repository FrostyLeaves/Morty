#include "MMaterialTemplateResourceData.h"
#include "MMaterialResource.h"
#include "Flatbuffer/MMaterial_generated.h"
#include "Flatbuffer/MMaterialTemplate_generated.h"
#include "Material/MMaterialPass.h"

using namespace morty;

flatbuffers::Offset<void> MMaterialTemplateResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    const auto                    fbShaderPath = fbb.CreateString(shaderPath);
    const auto                    fbMacro = shaderMacro.Serialize(fbb);

    // Serialize material passes
    std::vector<flatbuffers::Offset<fbs::MMaterialPass>> materialPassOffsets;
    for (const auto& [passName, pass] : materialPasses)
    {
        if (pass)
        {
            materialPassOffsets.push_back(pass->Serialize(fbb).o);
        }
    }
    auto materialPassVector = fbb.CreateVector(materialPassOffsets);

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
