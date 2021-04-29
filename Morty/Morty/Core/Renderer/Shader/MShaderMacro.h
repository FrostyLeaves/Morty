/**
 * @File         MShader
 * 
 * @Created      2020-5-8 22:15:46
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADER_MACRO_H_
#define _M_MSHADER_MACRO_H_
#include "MGlobal.h"
#include "MVariant.h"

#include <vector>

class MORTY_API MShaderMacro
{
public:

	void SetMacro(const MString& strKey, const MString& strValue);
    void AddUnionMacro(const MString& strKey, const MString& strValue = "");
    void RemoveMacro(const MString& strKey);

	bool Compare(const MShaderMacro& macro);

	void SetInnerMacro(const MString& strKey, const MString& strValue);
	MString GetInnerMacro(const MString& strKey);

	void WriteToStruct(MStruct& srt);
	void ReadFromStruct(const MStruct& srt);

protected:

	friend class MMaterial;
	void SetMacro(const MString& strKey, const MString& strValue, std::vector<std::pair<MString, MString> >& vector);

public:
	std::vector<std::pair<MString, MString> > m_vMacroParams;
	std::vector<std::pair<MString, MString> > m_vMortyMacroParams;

    static std::vector<std::pair<MString, MString> > s_vGlobalMacroParams;
};

#endif
