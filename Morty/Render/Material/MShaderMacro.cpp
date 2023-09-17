#include "MShaderMacro.h"

#include "Flatbuffer/MShaderMacro_generated.h"
#include "Render/MVertex.h"
#include "Utility/MFunction.h"

#include "Render/MRenderGlobal.h"

const MString strBonesPerVertex = MStringUtil::ToString(MRenderGlobal::BONES_PER_VERTEX);
const MString strBonesMaxNumber = MStringUtil::ToString(MRenderGlobal::BONES_MAX_NUMBER);
const MString strShadowTextureSize = MStringUtil::ToString(MRenderGlobal::SHADOW_TEXTURE_SIZE);
const MString strPointLightMaxNumber = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_MAX_NUMBER);
const MString strPointLightPixelNumber = MStringUtil::ToString(MRenderGlobal::POINT_LIGHT_PIXEL_NUMBER);
const MString strSpotLightMaxNumber = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_MAX_NUMBER);
const MString strSpotLightPixelNumber = MStringUtil::ToString(MRenderGlobal::SPOT_LIGHT_PIXEL_NUMBER);
const MString strCascadedShadowMapNumber = MStringUtil::ToString(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
const MString strMeshLODLevelRangeNumber = MStringUtil::ToString(MRenderGlobal::MESH_LOD_LEVEL_RANGE);
const MString strMergeInstancingMaxNumber = MStringUtil::ToString(MRenderGlobal::MERGE_INSTANCING_MAX_NUM);
const MString strTransformInUniformMaxNumber = MStringUtil::ToString(MRenderGlobal::MESH_TRANSFORM_IN_UNIFORM_MAX_NUM);
const MString strMergeInstancingClusterMaxNumber = MStringUtil::ToString(MRenderGlobal::MERGE_INSTANCING_CLUSTER_MAX_NUM);




enum class METransparentPolicy
{
	EDualDepthPeeling = 1,
};

const MString strTransparentPolicy = MStringUtil::ToString((int)METransparentPolicy::EDualDepthPeeling);

std::vector<std::pair<MString, MString>> MShaderMacro::s_vGlobalMacroParams = {
	{"MBONES_PER_VERTEX", strBonesPerVertex},
	{"MBONES_MAX_NUMBER", strBonesMaxNumber},
	{"MSHADOW_TEXTURE_SIZE", strShadowTextureSize},
	{"MCALC_NORMAL_IN_VS", MRenderGlobal::VERTEX_NORMAL ? "1" : "0"},
	{"MPOINT_LIGHT_MAX_NUMBER", strPointLightMaxNumber},
	{"MPOINT_LIGHT_PIXEL_NUMBER", strPointLightPixelNumber},
	{"MSPOT_LIGHT_MAX_NUMBER", strSpotLightMaxNumber},
	{"MSPOT_LIGHT_PIXEL_NUMBER", strSpotLightPixelNumber},
	{"MTRANSPARENT_POLICY", strTransparentPolicy},
	{"GBUFFER_UNIFIED_FORMAT", MRenderGlobal::GBUFFER_UNIFIED_FORMAT ? "1" : "0"},
	{"CASCADED_SHADOW_MAP_NUM", strCascadedShadowMapNumber},
	{"MESH_LOD_LEVEL_RANGE", strMeshLODLevelRangeNumber},
	{"MERGE_INSTANCING_MAX_NUM", strMergeInstancingMaxNumber},
	{"MERGE_INSTANCING_CLUSTER_MAX_NUM", strMergeInstancingClusterMaxNumber},
	{"MESH_TRANSFORM_IN_UNIFORM_MAX_NUM", strTransformInUniformMaxNumber},
};

void MShaderMacro::SetInnerMacro(const MString& strKey, const MString& strValue)
{
	SetMacro(strKey, strValue, m_vMortyMacroParams);
}

MString MShaderMacro::GetInnerMacro(const MString& strKey)
{
	for (auto pairs : m_vMortyMacroParams)
	{
		if (strKey == pairs.first)
			return pairs.second;
	}

	return MString();
}

void MShaderMacro::SetMacro(const MString& strKey, const MString& strValue)
{
	SetMacro(strKey, strValue, m_vMacroParams);
}

void MShaderMacro::SetMacro(const MString& strKey, const MString& strValue, std::vector<std::pair<MString, MString>>& vector)
{
	std::vector<std::pair<MString, MString>>::iterator iter = std::lower_bound(vector.begin(), vector.end(), strKey, [](const std::pair<MString, MString>& a, const MString& b) {
		return a.first < b;
		});

	if (iter == vector.end())
	{
		vector.push_back(std::pair<MString, MString>(strKey, strValue));
	}
	else if (iter->first == strKey)
	{
		iter->second = strValue;
	}
	else
	{
		vector.insert(iter, std::pair<MString, MString>(strKey, strValue));
	}
}

void MShaderMacro::AddUnionMacro(const MString& strKey, const MString& strValue /*= ""*/)
{
	std::pair<MString, MString> pair(strKey, strValue);

	UNION_ORDER_PUSH_BACK_VECTOR<std::pair<MString, MString>>(m_vMacroParams, pair
		, [](const std::pair<MString, MString>& a, const std::pair<MString, MString>& b) { return a.first < b.first; }
	, [](const std::pair<MString, MString>& a, const std::pair<MString, MString>& b) { return a.first == b.first; }
	);
}

void MShaderMacro::RemoveMacro(const MString& strKey)
{
	std::pair<MString, MString> pair(strKey, "");
	ERASE_UNION_ORDER_VECTOR<std::pair<MString, MString>>(m_vMacroParams, pair
		, [](const std::pair<MString, MString>& a, const std::pair<MString, MString>& b) { return a.first < b.first; }
	, [](const std::pair<MString, MString>& a, const std::pair<MString, MString>& b) { return a.first == b.first; }
	);
}

bool MShaderMacro::HasMacro(const MString& strKey)
{
	return FIND_ORDER_VECTOR<std::pair<MString, MString>, MString>(m_vMacroParams, strKey, [](const std::pair<MString, MString>& a, const MString& b) {
		return a.first < b;
	}) < m_vMacroParams.size();
}

bool MShaderMacro::Compare(const MShaderMacro& macro)
{
	uint32_t unSize = m_vMacroParams.size();
	uint32_t unMortySize = m_vMortyMacroParams.size();
	if (unSize != macro.m_vMacroParams.size())
		return false;
	if (unMortySize != macro.m_vMortyMacroParams.size())
		return false;

	for (uint32_t i = 0; i < unSize; ++i)
	{
		const std::pair<MString, MString>& a = m_vMacroParams[i];
		const std::pair<MString, MString>& b = macro.m_vMacroParams[i];

		if (a.first != b.first || a.second != b.second)
			return false;
	}

	for (uint32_t i = 0; i < unMortySize; ++i)
	{
		const std::pair<MString, MString>& a = m_vMortyMacroParams[i];
		const std::pair<MString, MString>& b = macro.m_vMortyMacroParams[i];

		if (a.first != b.first || a.second != b.second)
			return false;
	}

	return true;
}

flatbuffers::Offset<void> MShaderMacro::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<mfbs::MShaderMacroPair>> vMaterialMacroPairs;
	for (auto pairs : m_vMacroParams)
	{
		auto fbKey = fbb.CreateString(pairs.first);
		auto fbValue = fbb.CreateString(pairs.second);
		mfbs::MShaderMacroPairBuilder builder(fbb);
		builder.add_key(fbKey);
		builder.add_value(fbValue);
		vMaterialMacroPairs.push_back(builder.Finish().o);
	}

	std::vector<flatbuffers::Offset<mfbs::MShaderMacroPair>> vInnerMacroPairs;
	for (auto pairs : m_vMortyMacroParams)
	{
		auto fbKey = fbb.CreateString(pairs.first);
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
			m_vMacroParams.push_back({ pair->key()->c_str(), pair->value()->c_str() });
		}
	}

	if (fbData->inner_macro())
	{
		for (auto pair : *fbData->inner_macro())
		{
			m_vMortyMacroParams.push_back({ pair->key()->c_str(), pair->value()->c_str() });
		}
	}
}
