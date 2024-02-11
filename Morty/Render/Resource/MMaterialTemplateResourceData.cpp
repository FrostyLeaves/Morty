#include "MMaterialTemplateResourceData.h"
#include "MMaterialResource.h"
#include "Flatbuffer/MMaterial_generated.h"

flatbuffers::Offset<void> MMaterialTemplateResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	const auto fbVertexShader = fbb.CreateString(vShaders[size_t(MEShaderType::EVertex)]);
	const auto fbPixelShader = fbb.CreateString(vShaders[size_t(MEShaderType::EPixel)]);
	const auto fbMacro = shaderMacro.Serialize(fbb);

	morty::MMaterialTemplateBuilder builder(fbb);

	builder.add_vertex_resource(fbVertexShader.o);
	builder.add_pixel_resource(fbPixelShader.o);
	builder.add_material_macro(fbMacro.o);
	builder.add_material_type(static_cast<morty::MEMaterialType>(eMaterialType));
	builder.add_rasterizer_type(static_cast<morty::MECullMode>(eCullMode));

	return builder.Finish().Union();
}

void MMaterialTemplateResourceData::Deserialize(const void* pBufferPointer)
{
	const morty::MMaterialTemplate* fbData =morty::GetMMaterialTemplate(pBufferPointer);

	eMaterialType = static_cast<MEMaterialType>(fbData->material_type());
	eCullMode = static_cast<MECullMode>(fbData->rasterizer_type());

	shaderMacro.Deserialize(fbData->material_macro());

	if (fbData->vertex_resource())
	{
		vShaders[size_t(MEShaderType::EVertex)] = fbData->vertex_resource()->str();
	}
	if (fbData->pixel_resource())
	{
		vShaders[size_t(MEShaderType::EPixel)] = fbData->pixel_resource()->str();
	}
}
