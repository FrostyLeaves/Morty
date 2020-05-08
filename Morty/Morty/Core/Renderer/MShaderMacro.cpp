#include "MShaderMacro.h"

void MShaderMacro::SetMacro(const MString& strKey, const MString& strValue)
{
	std::vector<std::pair<MString, MString>>::iterator iter = std::lower_bound(m_vMacroParams.begin(), m_vMacroParams.end(), strKey, [](const std::pair<MString, MString>& a, const MString& b) {
		return a.first < b;
		});

	if (iter == m_vMacroParams.end())
	{
		m_vMacroParams.push_back(std::pair<MString, MString>(strKey, strValue));
	}
	else if (iter->first == strKey)
	{
		iter->second = strValue;
	}
	else
	{
		m_vMacroParams.insert(iter, std::pair<MString, MString>(strKey, strValue));
	}

}

bool MShaderMacro::Compare(const MShaderMacro& macro)
{
	unsigned int unSize = m_vMacroParams.size();
	if (unSize != macro.m_vMacroParams.size())
		return false;

	for (unsigned int i = 0; i < unSize; ++i)
	{
		const std::pair<MString, MString>& a = m_vMacroParams[i];
		const std::pair<MString, MString>& b = macro.m_vMacroParams[i];

		if (a.first != b.first || a.second != b.second)
			return false;
	}

	return true;
}
