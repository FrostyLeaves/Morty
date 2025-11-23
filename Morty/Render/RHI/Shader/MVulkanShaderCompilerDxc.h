/**
 * @File         MVulkanShaderCompiler
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MRenderGlobal.h"
#include "MVulkanShaderCompiler.h"
#include "Shader/MShader.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

namespace morty
{

class MORTY_API MVulkanShaderCompilerDxc : public MVulkanShaderCompiler
{
public:
    explicit MVulkanShaderCompilerDxc(MVulkanDevice* pDevice);

    bool CompileShader(
            const MString&         strShaderPath,
            const MStringId&       strEntryName,
            const MEShaderType&    eShaderType,
            const MShaderMacro&    macro,
            std::vector<uint32_t>& vSpirv
    ) override;
};

}// namespace morty

#endif
