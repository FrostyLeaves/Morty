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

	bool Compare(const MShaderMacro& macro);

public:

	std::vector<std::pair<MString, MString>> m_vMacroParams;

};

#endif
