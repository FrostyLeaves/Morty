#include "MMaterialResourceData.h"
#include "MMaterialResource.h"
#include "Flatbuffer/MMaterial_generated.h"

flatbuffers::Offset<void> MMaterialResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	const auto fbVertexShader = fbb.CreateString(vShaders[size_t(MEShaderType::EVertex)]);
	const auto fbPixelShader = fbb.CreateString(vShaders[size_t(MEShaderType::EPixel)]);
	const auto fbMacro = shaderMacro.Serialize(fbb);


	std::vector<flatbuffers::Offset<mfbs::MMaterialTexture>> fbTextureOffsets;
	for (const auto& texture : vTextures)
	{
		const auto fbName = fbb.CreateString(texture.name);
		const auto fbTexture = fbb.CreateString(texture.value);

		mfbs::MMaterialTextureBuilder builder(fbb);
		builder.add_name(fbName);
		builder.add_texture(fbTexture.o);
		fbTextureOffsets.push_back(builder.Finish());
	}

	std::vector<flatbuffers::Offset<mfbs::MMaterialProperty>> fbPropertyOffsets;
	for (const auto& param : vProperty)
	{
		const auto fbName = fbb.CreateString(param.name);
		const auto fbProperty = param.value.Serialize(fbb);

		mfbs::MMaterialPropertyBuilder builder(fbb);
		builder.add_name(fbName);
		builder.add_property(fbProperty.o);
		fbPropertyOffsets.push_back(builder.Finish());
	}

	const auto fbTextures = fbb.CreateVector(fbTextureOffsets);
	const auto fbProperty = fbb.CreateVector(fbPropertyOffsets);

	mfbs::MMaterialBuilder builder(fbb);

	builder.add_vertex_resource(fbVertexShader.o);
	builder.add_pixel_resource(fbPixelShader.o);
	builder.add_material_macro(fbMacro.o);
	builder.add_material_type(static_cast<mfbs::MEMaterialType>(eMaterialType));
	builder.add_rasterizer_type(static_cast<mfbs::MECullMode>(eCullMode));
	builder.add_material_textures(fbTextures);
	builder.add_material_property(fbProperty);

	return builder.Finish().Union();
}

void MMaterialResourceData::Deserialize(const void* pBufferPointer)
{
	const mfbs::MMaterial* fbData = mfbs::GetMMaterial(pBufferPointer);

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

	if (fbData->material_property())
	{
		const size_t nPropertyNum = fbData->material_property()->size();
		vProperty.resize(nPropertyNum);
		for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
		{
			const auto fbProperty = fbData->material_property()->Get(nIdx);
			vProperty[nIdx].name = fbProperty->name()->c_str();
			vProperty[nIdx].value.Deserialize(fbProperty->property());
		}
	}

	if (fbData->material_textures())
	{
		const size_t nTextureNum = fbData->material_textures()->size();
		vTextures.resize(nTextureNum);
		for (size_t nIdx = 0; nIdx < nTextureNum; ++nIdx)
		{
			const auto fbTexture = fbData->material_textures()->Get(nIdx);
			vTextures[nIdx].name = fbTexture->name()->c_str();
			vTextures[nIdx].value = fbTexture->texture()->str();
		}
	}
}
