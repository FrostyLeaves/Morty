#include "MShaderMacro.h"
#include "Render/MVertex.h"
#include "Utility/MFunction.h"

#include "Render/MRenderGlobal.h"

const MString strBonesPerVertex = MStringHelper::ToString(MRenderGlobal::BONES_PER_VERTEX);
const MString strBonesMaxNumber = MStringHelper::ToString(MRenderGlobal::BONES_MAX_NUMBER);
const MString strShadowTextureSize = MStringHelper::ToString(MRenderGlobal::SHADOW_TEXTURE_SIZE);
const MString strPointLightMaxNumber = MStringHelper::ToString(MRenderGlobal::POINT_LIGHT_MAX_NUMBER);
const MString strPointLightPixelNumber = MStringHelper::ToString(MRenderGlobal::POINT_LIGHT_PIXEL_NUMBER);
const MString strSpotLightMaxNumber = MStringHelper::ToString(MRenderGlobal::SPOT_LIGHT_MAX_NUMBER);
const MString strSpotLightPixelNumber = MStringHelper::ToString(MRenderGlobal::SPOT_LIGHT_PIXEL_NUMBER);
const MString strCascadedShadowMapNumber = MStringHelper::ToString(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);


enum class METransparentPolicy
{
	EDualDepthPeeling = 1,
};

const MString strTransparentPolicy = MStringHelper::ToString((int)METransparentPolicy::EDualDepthPeeling);

std::vector<std::pair<MString, MString>> MShaderMacro::s_vGlobalMacroParams = {
	{"MBONES_PER_VERTEX", strBonesPerVertex},
	{"MBONES_MAX_NUMBER", strBonesMaxNumber},
	{"MSHADOW_TEXTURE_SIZE", strShadowTextureSize},
	{"MCALC_NORMAL_IN_VS", MRenderGlobal::VERTEX_NORMAL ? "true" : "false"},
	{"MPOINT_LIGHT_MAX_NUMBER", strPointLightMaxNumber},
	{"MPOINT_LIGHT_PIXEL_NUMBER", strPointLightPixelNumber},
	{"MSPOT_LIGHT_MAX_NUMBER", strSpotLightMaxNumber},
	{"MSPOT_LIGHT_PIXEL_NUMBER", strSpotLightPixelNumber},
	{"MTRANSPARENT_POLICY", strTransparentPolicy},
	{"GBUFFER_UNIFIED_FORMAT", MRenderGlobal::GBUFFER_UNIFIED_FORMAT ? "true" : "false"},
	{"CASCADED_SHADOW_MAP_NUM", strCascadedShadowMapNumber},
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

bool MShaderMacro::Compare(const MShaderMacro& macro)
{
	uint32_t unSize = m_vMacroParams.size();
	uint32_t unMortySize = m_vMacroParams.size();
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

void MShaderMacro::WriteToStruct(MStruct& srt)
{
	srt.SetValue("mat", MVariantArray());
	if (MVariantArray* pArray = srt.GetValue<MVariantArray>("mat"))
	{
		for (auto pairs : m_vMacroParams)
		{
			pArray->AppendValue(pairs.first);
			pArray->AppendValue(pairs.second);
		}
	}

	srt.SetValue("morty", MVariantArray());
	if (MVariantArray* pArray = srt.GetValue<MVariantArray>("morty"))
	{
		for (auto pairs : m_vMortyMacroParams)
		{
			pArray->AppendValue(pairs.first);
			pArray->AppendValue(pairs.second);
		}
	}
}

void MShaderMacro::ReadFromStruct(const MStruct& srt)
{
	m_vMacroParams.clear();
	m_vMortyMacroParams.clear();

	if (const MVariantArray* pArray = srt.GetValue<MVariantArray>("mat"))
	{
		for (uint32_t i = 0; i < pArray->GetMemberCount(); i+=2)
		{
			const MString* key = (*pArray)[i].GetString();
			const MString* value = (*pArray)[i + 1].GetString();

			m_vMacroParams.push_back({ *key, *value });
		}
	}

	if (const MVariantArray* pArray = srt.GetValue<MVariantArray>("morty"))
	{
		for (uint32_t i = 0; i < pArray->GetMemberCount(); i += 2)
		{
			const MString* key = (*pArray)[i].GetString();
			const MString* value = (*pArray)[i + 1].GetString();

			m_vMortyMacroParams.push_back({ *key, *value });
		}
	}
}
