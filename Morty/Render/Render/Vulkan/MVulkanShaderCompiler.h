/**
 * @File         MVulkanShaderCompiler
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Shader/MShader.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_parser.hpp"

MORTY_SPACE_BEGIN

class MShaderBuffer;
class MVulkanDevice;
class MVertexShaderBuffer;
class MORTY_API MPreamble {
public:
	MPreamble();
	~MPreamble();

	bool IsValid() const;
	const char* GetText() const { return m_strText.c_str(); }

	const std::vector<std::string>& GetProcesses() { return m_vProcesses; }

	void AddDef(const MString& strName, const MString& strValue);

	void AddUndef(std::string undef);


private:
	std::vector<std::string> m_vProcesses;
	std::string m_strText;  // contents of preamble
};

class MORTY_API MVulkanShaderCompiler
{
public:
    MVulkanShaderCompiler(MVulkanDevice* pDevice);
	~MVulkanShaderCompiler();

    bool CompileShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);

	void ConvertMacro(const MShaderMacro& macro, MPreamble& preamble);
	
private:

	bool CompileHlslShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);

	MVulkanDevice* m_pDevice;
};

MORTY_SPACE_END

#endif
