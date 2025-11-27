//
// Created by DoubleYe on 2025/3/22.
//

#ifndef MORTY_MSLANGCOMPILER_H
#define MORTY_MSLANGCOMPILER_H

#include "Utility/MGlobal.h"
#include "Shader/MShader.h"
#include "Shader/MShaderPropertyBlock.h"
#include "Utility/MString.h"
#include "Utility/MStringId.h"


namespace morty
{

class ISlangCompilerSession
{
};

struct MSlangOutput {
    MStringId             name;
    MEShaderType          type;
    std::vector<uint32_t> buffer;
};

class MSlangCompiler
{

public:
    void                                           SetShaderPath(MStringView filePath);
    bool                                           Compile();

    [[nodiscard]] const std::vector<MSlangOutput>& GetOutput() const { return m_output; }
    [[nodiscard]] const MShaderPropertyBlock&      GetReflection() const { return m_reflection; }

private:
    MString                                       m_filePath;

    std::vector<MSlangOutput>                     m_output;
    MShaderPropertyBlock                          m_reflection;
    static std::unique_ptr<ISlangCompilerSession> s_globalSession;
};

}// namespace morty
#endif//MORTY_MSLANGCOMPILER_H
