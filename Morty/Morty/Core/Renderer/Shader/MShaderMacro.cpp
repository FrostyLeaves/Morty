﻿#include "MShaderMacro.h"
#include "MVertex.h"
#include "MFunction.h"

const MString strBonesPerVertex = MStringHelper::ToString(MBONES_PER_VERTEX);
const MString strBonesMaxNumber = MStringHelper::ToString(MBONES_MAX_NUMBER);
const MString strShadowTextureSize = MStringHelper::ToString(MGlobal::MSHADOW_TEXTURE_SIZE);
const MString strPointLightMaxNumber = MStringHelper::ToString(MGlobal::MPOINT_LIGHT_MAX_NUMBER);
const MString strPointLightPixelNumber = MStringHelper::ToString(MGlobal::MPOINT_LIGHT_PIXEL_NUMBER);
const MString strSpotLightMaxNumber = MStringHelper::ToString(MGlobal::MSPOT_LIGHT_MAX_NUMBER);
const MString strSpotLightPixelNumber = MStringHelper::ToString(MGlobal::MSPOT_LIGHT_PIXEL_NUMBER);


enum class METransparentPolicy
{
	EDualDepthPeeling = 1,
};

const MString strTransparentPolicy = MStringHelper::ToString((int)METransparentPolicy::EDualDepthPeeling);

std::vector<std::pair<MString, MString>> MShaderMacro::s_vGlobalMacroParams = {
	{"MBONES_PER_VERTEX", strBonesPerVertex},
	{"MBONES_MAX_NUMBER", strBonesMaxNumber},
	{"MSHADOW_TEXTURE_SIZE", strShadowTextureSize},
	{"MCALC_NORMAL_IN_VS", MGlobal::MCALC_NORMAL_IN_VS ? "true" : "false"},
	{"MPOINT_LIGHT_MAX_NUMBER", strPointLightMaxNumber},
	{"MPOINT_LIGHT_PIXEL_NUMBER", strPointLightPixelNumber},
	{"MSPOT_LIGHT_MAX_NUMBER", strSpotLightMaxNumber},
	{"MSPOT_LIGHT_PIXEL_NUMBER", strSpotLightPixelNumber},
	{"MTRANSPARENT_POLICY", strTransparentPolicy},
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
	if (MVariantArray* pArray = srt.AppendMVariant<MVariantArray>("mat"))
	{
		for (auto pairs : m_vMacroParams)
		{
			pArray->AppendMVariant(pairs.first);
			pArray->AppendMVariant(pairs.second);
		}
	}
	
	if (MVariantArray* pArray = srt.AppendMVariant<MVariantArray>("morty"))
	{
		for (auto pairs : m_vMortyMacroParams)
		{
			pArray->AppendMVariant(pairs.first);
			pArray->AppendMVariant(pairs.second);
		}
	}
}

void MShaderMacro::ReadFromStruct(MStruct& srt)
{
	m_vMacroParams.clear();
	m_vMortyMacroParams.clear();

	if (MVariantArray* pArray = srt.FindMember<MVariantArray>("mat"))
	{
		for (uint32_t i = 0; i < pArray->GetMemberCount(); i+=2)
		{
			MString* key = (*pArray)[i].GetString();
			MString* value = (*pArray)[i + 1].GetString();

			m_vMacroParams.push_back({ *key, *value });
		}
	}

	if (MVariantArray* pArray = srt.FindMember<MVariantArray>("morty"))
	{
		for (uint32_t i = 0; i < pArray->GetMemberCount(); i += 2)
		{
			MString* key = (*pArray)[i].GetString();
			MString* value = (*pArray)[i + 1].GetString();

			m_vMortyMacroParams.push_back({ *key, *value });
		}
	}
}
