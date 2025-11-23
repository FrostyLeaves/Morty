/**
 * @File         MVulkanShaderCompiler
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       DoubleYe
**/

#pragma once
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Utility/MRenderGlobal.h"
#include "Shader/MShader.h"

namespace morty
{

class MShaderBuffer;
class MVulkanDevice;
class MVertexShaderBuffer;
class MORTY_API MPreamble
{
public:
    MPreamble();

    ~MPreamble();

    [[nodiscard]] bool              IsValid() const;

    [[nodiscard]] const char*       GetText() const { return m_strText.c_str(); }

    const std::vector<std::string>& GetProcesses() { return m_processes; }

    void                            AddDef(const MString& strName, const MString& strValue);

    void                            AddUndef(std::string undef);


private:
    std::vector<std::string> m_processes;
    std::string              m_strText;// contents of preamble
};

class MORTY_API MVulkanShaderCompiler
{
public:
    explicit MVulkanShaderCompiler(MVulkanDevice* pDevice);
    virtual ~MVulkanShaderCompiler() = default;

    virtual bool CompileShader(
            const MString&         strShaderPath,
            const MStringId&       strEntryName,
            const MEShaderType&    eShaderType,
            const MShaderMacro&    macro,
            std::vector<uint32_t>& vSpirv
    ) = 0;

    void           ConvertMacro(const MShaderMacro& macro, MPreamble& preamble);

    MVulkanDevice* GetDevice() const { return m_device; }

private:
    MVulkanDevice* m_device;
};

}// namespace morty

#endif
