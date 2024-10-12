/**
 * @File         MShader
 * 
 * @Created      2019-08-26 21:24:51
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

#include "MShaderBuffer.h"
#include "MShaderMacro.h"

namespace morty
{

enum class MEShaderType
{
    EVertex   = 0,
    EPixel    = 1,
    ECompute  = 2,
    EGeometry = 3,

    TOTAL_NUM,
    ENone = 0xffff,
};


class MIDevice;
class MShaderBuffer;
class MORTY_API MShader
{
public:
    MShader() = default;

    virtual ~MShader() = default;

    bool                CompileShader(MIDevice* pDevice);

    void                CleanShader(MIDevice* pDevice);

    MEShaderType        GetType() const { return m_shaderType; }

    const MShaderMacro& GetMacro() { return m_ShaderMacro; }

    const MString&      GetShaderPath() { return m_strShaderPath; }

    void                SetBuffer(MShaderBuffer* pShaderBuffer) { m_shaderBuffer = pShaderBuffer; }

    MShaderBuffer*      GetBuffer() { return m_shaderBuffer; }

private:
    friend class MShaderResource;

    MShaderMacro   m_ShaderMacro;
    MString        m_strShaderPath;
    MEShaderType   m_shaderType   = MEShaderType::ENone;
    MShaderBuffer* m_shaderBuffer = nullptr;
};

}// namespace morty