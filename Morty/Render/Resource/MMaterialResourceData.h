/**
 * @File         MMaterialResourceData
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MMaterialResource.h"

struct MORTY_API MMaterialResourceData : public MFbResourceData
{
	struct Property
	{
		MString name;
		MVariant value;
	};

	struct Texture
	{
		MString name;
		MPath value;
	};

	//RawData
	MPath vertexShader;
	MPath pixelShader;
	MShaderMacro shaderMacro;
	MEMaterialType eMaterialType;
	MECullMode eCullMode;

	std::vector<Property> vProperty;
	std::vector<Texture> vTextures;

	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;
	void Deserialize(const void* pBufferPointer) override;
	
};


class MORTY_API MMaterialResourceLoader : public MResourceLoaderTemplate<MMaterialResource, MMaterialResourceData>
{
public:
	static MString GetResourceTypeName() { return "Material"; }
	static std::vector<MString> GetSuffixList() { return { "mat" }; }
};
