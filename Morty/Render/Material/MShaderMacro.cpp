#include "MShaderMacro.h"

#include "Flatbuffer/MShaderMacro_generated.h"
#include "Render/MVertex.h"
#include "Utility/MFunction.h"

#include "Render/MRenderGlobal.h"

enum class METransparentPolicy
{
	EDualDepthPeeling = 1,
};

const MString strBonesPerVertex = MStringUtil::ToString(MRenderGlobal::BONES_PER_VERTEX);
const MString strPointLightMaxNumber = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_MAX_NUMBER);
const MString strPointLightPixelNumber = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_PIXEL_NUMBER);
const MString strSpotLightMaxNumber = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_MAX_NUMBER);
const MString strSpotLightPixelNumber = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_PIXEL_NUMBER);
const MString strCascadedShadowMapNumber = MStringUtil::ToString(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
const MString strMeshLODLevelRangeNumber = MStringUtil::ToString(MRenderGlobal::MESH_LOD_LEVEL_RANGE);
const MString strTransformInUniformMaxNumber = MStringUtil::ToString(MRenderGlobal::MESH_TRANSFORM_IN_UNIFORM_MAX_NUM);
const MString strTransparentPolicy = MStringUtil::ToString((int)METransparentPolicy::EDualDepthPeeling);
const MString strVoxelClipMapNumber = MStringUtil::ToString(MRenderGlobal::VOXEL_GI_CLIP_MAP_NUM);
const MString strVoxelDiffuseConeCount = MStringUtil::ToString(16);

std::unordered_map<MStringId, MString> MShaderMacro::s_vGlobalMacroParams = {
	{MStringId("MBONES_PER_VERTEX"), strBonesPerVertex},
	{MStringId("MPOINT_LIGHT_MAX_NUMBER"), strPointLightMaxNumber},
	{MStringId("MPOINT_LIGHT_PIXEL_NUMBER"), strPointLightPixelNumber},
	{MStringId("MSPOT_LIGHT_MAX_NUMBER"), strSpotLightMaxNumber},
	{MStringId("MSPOT_LIGHT_PIXEL_NUMBER"), strSpotLightPixelNumber},
	{MStringId("MTRANSPARENT_POLICY"), strTransparentPolicy},
	{MStringId("CASCADED_SHADOW_MAP_NUM"), strCascadedShadowMapNumber},
	{MStringId("MESH_LOD_LEVEL_RANGE"), strMeshLODLevelRangeNumber},
	{MStringId("MESH_TRANSFORM_IN_UNIFORM_MAX_NUM"), strTransformInUniformMaxNumber},
	{MStringId("VOXEL_GI_CLIP_MAP_NUM"), strVoxelClipMapNumber},
	{MStringId("VOXEL_DIFFUSE_CONE_COUNT"), strVoxelDiffuseConeCount},
};

void MShaderMacro::SetInnerMacro(const MStringId& strKey, const MString& strValue)
{
	SetMacro(strKey, strValue, m_vMortyMacroParams);
}

MString MShaderMacro::GetInnerMacro(const MStringId& strKey)
{
	auto findResult = m_vMortyMacroParams.find(strKey);
	if (findResult == m_vMortyMacroParams.end())
	{
		return {};
	}

	return findResult->second;
}

void MShaderMacro::SetMacro(const MStringId& strKey, const MString& strValue)
{
	SetMacro(strKey, strValue, m_vMacroParams);
}

void MShaderMacro::SetMacro(const MStringId& strKey, const MString& strValue, std::unordered_map<MStringId, MString>& table)
{
	table[strKey] = strValue;
}

void MShaderMacro::AddUnionMacro(const MStringId& strKey, const MString& strValue /*= ""*/)
{
	MORTY_ASSERT(m_vMacroParams.find(strKey) == m_vMacroParams.end());
	m_vMacroParams[strKey] = strValue;
}

void MShaderMacro::RemoveMacro(const MStringId& strKey)
{
	m_vMacroParams.erase(strKey);
}

bool MShaderMacro::HasMacro(const MStringId& strKey)
{
	return m_vMacroParams.find(strKey) != m_vMacroParams.end();
}

bool MShaderMacro::Compare(const MShaderMacro& macro)
{
	if (m_vMacroParams != macro.m_vMacroParams)
	{
		return false;
	}

	if (m_vMortyMacroParams != macro.m_vMortyMacroParams)
	{
		return false;
	}

	return true;
}

flatbuffers::Offset<void> MShaderMacro::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MShaderMacroPair>> vMaterialMacroPairs;
	for (auto pairs : m_vMacroParams)
	{
		auto fbKey = fbb.CreateString(pairs.first.ToString());
		auto fbValue = fbb.CreateString(pairs.second);
		mfbs::MShaderMacroPairBuilder builder(fbb);
		builder.add_key(fbKey);
		builder.add_value(fbValue);
		vMaterialMacroPairs.push_back(builder.Finish().o);
	}

	std::vector<flatbuffers::Offset<mfbs::MShaderMacroPair>> vInnerMacroPairs;
	for (auto pairs : m_vMortyMacroParams)
	{
		auto fbKey = fbb.CreateString(pairs.first.ToString());
		auto fbValue = fbb.CreateString(pairs.second);
		mfbs::MShaderMacroPairBuilder builder(fbb);
		builder.add_key(fbKey);
		builder.add_value(fbValue);
		vInnerMacroPairs.push_back(builder.Finish().o);
	}

	const auto fbMaterialMacro = fbb.CreateVector(vMaterialMacroPairs);
	const auto fbInnerMacro = fbb.CreateVector(vInnerMacroPairs);
	mfbs::MShaderMacroBuilder builder(fbb);

	builder.add_material_macro(fbMaterialMacro);
	builder.add_inner_macro(fbInnerMacro);

	return builder.Finish().Union();
}

void MShaderMacro::Deserialize(const void* pBufferPointer)
{
	const mfbs::MShaderMacro* fbData = reinterpret_cast<const mfbs::MShaderMacro*>(pBufferPointer);

	m_vMacroParams.clear();
	m_vMortyMacroParams.clear();

	if (!pBufferPointer)
	{
		return;
	}

	if (fbData->material_macro())
	{
		for (auto pair : *fbData->material_macro())
		{
			m_vMacroParams[MStringId(pair->key()->c_str())] = pair->value()->c_str();
		}
	}

	if (fbData->inner_macro())
	{
		for (auto pair : *fbData->inner_macro())
		{
			m_vMortyMacroParams[MStringId(pair->key()->c_str())] = pair->value()->c_str();
		}
	}
}
