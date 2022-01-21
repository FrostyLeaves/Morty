/**
 * @File         MVulkanShaderCompiler
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       DoubleYe
**/

#ifndef _M_VULKANMSHADERCOMPILER_H_
#define _M_VULKANMSHADERCOMPILER_H_
#include "MRenderGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "MShader.h"

#ifdef max
#undef max
#endif

#include "spirv_cross.hpp"
#include "spirv_parser.hpp"

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
	void AddDef2(const MString& strName, const MString& strValue);

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

public:

    bool Initialize();

    bool CompileShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);

	void GetVertexInputState(const spirv_cross::Compiler& compiler, MVertexShaderBuffer* pShaderBuffer);

public:

	void GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer);

	void ConvertMacro(const MShaderMacro& macro, MPreamble& preamble);

	void ConvertMacroDXC(const MShaderMacro& macro, MPreamble& preamble);

	void ConvertVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant);

	bool ResetVariantType(const spirv_cross::SPIRType& type, MVariant& variant);

	void ReadShaderPath(const MString& strShaderPath);


private:

	bool CompileGlslShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);
	bool CompileHlslShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);

private:
	struct TBuiltInResource* m_pDefaultBuiltInResource;

	MVulkanDevice* m_pDevice;
};


#endif


#endif