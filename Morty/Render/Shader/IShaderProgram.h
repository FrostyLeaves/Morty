#pragma once

#include "Utility/MRenderGlobal.h"
#include "Shader/MShader.h"
#include "Shader/MShaderMacro.h"
#include "Type/MType.h"

namespace morty
{

class Shader;
class MResource;
class MShaderPropertyBlock;

class MORTY_API IShaderProgram : public MTypeClass
{
public:
    enum class EUsage
    {
        EUnknow,
        EGraphics,
        ECompute,
    };

public:
    MORTY_INTERFACE(IShaderProgram)

    virtual const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>&
                                                     GetShaderPropertyBlocks() = 0;

    virtual MShaderMacro&                            GetShaderMacro()                            = 0;
    [[nodiscard]] virtual MStringId                  GetEntryName(MEShaderType shaderType) const = 0;
    [[nodiscard]] virtual std::shared_ptr<MResource> GetShaderResource() const                   = 0;
    virtual MShader*                                 GetShader(MEShaderType)                     = 0;

    virtual MHashCode                                GetHashCode() const = 0;
    virtual bool                                     IsValid() const     = 0;
};

}// namespace morty