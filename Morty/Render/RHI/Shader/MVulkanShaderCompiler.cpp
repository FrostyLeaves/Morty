#include "MVulkanShaderCompiler.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanDevice.h"

using namespace morty;

MVulkanShaderCompiler::MVulkanShaderCompiler(MVulkanDevice* pDevice)
    : m_device(pDevice)
{}

MPreamble::MPreamble() {}

MPreamble::~MPreamble() {}

bool MPreamble::IsValid() const { return m_strText.size() > 0; }

void MPreamble::AddDef(const MString& strName, const MString& strValue)
{
#ifdef MORTY_SHADER_COMPILER_DXC
    if (strValue.empty())
    {
        m_strText.append(strName);
        m_strText.append(" ");
    }
    else { m_strText.append(strName + "=" + strValue + " "); }
#else
    m_strText.append("#define ");

    m_processes.push_back("define-macro ");
    if (strValue.empty())
    {
        m_processes.back().append(strName);
        m_strText.append(strName);
        m_strText.append("\n");
    }
    else
    {
        m_processes.back().append(strName + "=" + strValue);
        m_strText.append(strName + " " + strValue + "\n");
    }
#endif
}

void MPreamble::AddUndef(std::string undef)
{
    m_strText.append("#undef ");

    m_processes.push_back("undef-macro ");
    m_processes.back().append(undef);

    m_strText.append(undef);
    m_strText.append("\n");
}

void MVulkanShaderCompiler::ConvertMacro(const MShaderMacro& macro, MPreamble& preamble)
{
    for (const auto& m: macro.s_vGlobalMacroParams) preamble.AddDef(m.first.ToString(), m.second);

    for (const auto& m: macro.m_mortyMacroParams) preamble.AddDef(m.first.ToString(), m.second);

    for (const auto& m: macro.m_macroParams) preamble.AddDef(m.first.ToString(), m.second);
}


#endif
