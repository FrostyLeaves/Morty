﻿/**
 * @File         MShader
 * 
 * @Created      2019-08-26 21:24:51
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#include "MShaderMacro.h"
#include "MShaderBuffer.h"

enum class MEShaderType
{
	ENone = 0,
	EVertex = 1,
	EPixel = 2,
	ECompute = 3,
};


class MIDevice;
class MShaderBuffer;
class MORTY_API MShader
{
public:
    MShader() = default;
    virtual ~MShader() = default;

	bool CompileShader(MIDevice* pDevice);
	void CleanShader(MIDevice* pDevice);

	MEShaderType GetType() const { return m_eShaderType; }
	const MShaderMacro& GetMacro() { return m_ShaderMacro; }
	const MString& GetShaderPath() { return m_strShaderPath; }

	void SetBuffer(MShaderBuffer* pShaderBuffer) { m_pShaderBuffer = pShaderBuffer; }
	MShaderBuffer* GetBuffer() { return m_pShaderBuffer; }

private:

	friend class MShaderResource;
	MShaderMacro m_ShaderMacro;
	MString m_strShaderPath;
	MEShaderType m_eShaderType = MEShaderType::ENone;
	MShaderBuffer* m_pShaderBuffer = nullptr;

};
