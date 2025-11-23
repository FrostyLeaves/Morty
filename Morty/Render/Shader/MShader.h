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


enum class MEShaderLanguageType
{
    HLSL = 0,
    Slang,
};

struct MORTY_API MEntryNameGroup
{
    MEntryNameGroup()  = default;
    ~MEntryNameGroup() = default;

    std::array<MStringId, 4> array;
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

    const MStringId& GetEntryName() const { return m_entryName; }

    [[nodiscard]] MEShaderType GetShaderType() const { return m_shaderType; }

    [[nodiscard]] MEShaderLanguageType GetLanguageType() const { return m_languageType; }

    const MShaderMacro& GetMacro() { return m_ShaderMacro; }

    const MString& GetShaderPath() { return m_strShaderPath; }

    void SetBuffer(MShaderBuffer* shaderBuffer);

    [[nodiscard]] MShaderBuffer* GetBuffer() const;

    [[nodiscard]] bool IsCompiled() const { return m_compiled; }

private:
    friend class MShaderResource;

    MShaderMacro         m_ShaderMacro;
    MStringId            m_entryName;
    MString              m_strShaderPath;
    MEShaderType         m_shaderType   = MEShaderType::EVertex;
    MEShaderLanguageType m_languageType = MEShaderLanguageType::HLSL;
    MShaderBuffer*       m_shaderBuffer = nullptr;
    bool                 m_compiled     = false;
};

}// namespace morty