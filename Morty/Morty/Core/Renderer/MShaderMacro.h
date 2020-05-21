/**
 * @File         MShader
 * 
 * @Created      2020-5-8 22:15:46
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADER_MACRO_H_
#define _M_MSHADER_MACRO_H_
#include "MGlobal.h"

#include <vector>

class MORTY_CLASS MShaderMacro
{
public:

	void SetMacro(const MString& strKey, const MString& strValue);
    void AddUnionMacro(const MString& strKey, const MString& strValue = "");
    void RemoveMacro(const MString& strKey);

	bool Compare(const MShaderMacro& macro);


protected:

    friend class MMaterial;
    void SetMortyMacro(const MString& strKey, const MString& strValue);
	void SetMacro(const MString& strKey, const MString& strValue, std::vector<std::pair<MString, MString>>& vector);

public:
	std::vector<std::pair<MString, MString>> m_vMacroParams;
    
	std::vector<std::pair<MString, MString>> m_vMortyMacroParams;
    static std::vector<std::pair<MString, MString>> s_vGlobalMacroParams;
};

#endif
