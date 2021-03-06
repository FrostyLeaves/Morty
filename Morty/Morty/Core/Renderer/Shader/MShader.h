﻿/**
 * @File         MShader
 * 
 * @Created      2019-08-26 21:24:51
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADER_H_
#define _M_MSHADER_H_
#include "MGlobal.h"
#include "MString.h"
#include "MVertex.h"
#include "MVariant.h"
#include "MShaderMacro.h"
#include "MRenderStructure.h"

class MIDevice;
class MShaderBuffer;
class MORTY_API MShader
{
public:
    MShader();
    virtual ~MShader();

	bool CompileShader(MIDevice* pDevice);
	void CleanShader(MIDevice* pDevice);
	MShaderBuffer* GetBuffer() { return m_pShaderBuffer; }

	const MEShaderType GetType() const { return m_eShaderType; }

private:

	friend class MShaderResource;
	MShaderMacro m_ShaderMacro;
	MString m_strShaderPath;
	MEShaderType m_eShaderType;

	MShaderBuffer* m_pShaderBuffer;

};


#endif
