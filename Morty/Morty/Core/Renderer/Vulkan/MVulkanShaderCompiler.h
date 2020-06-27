/**
 * @File         MVulkanShaderCompiler
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       Pobrecito
**/

#ifndef _M_VULKANMSHADERCOMPILER_H_
#define _M_VULKANMSHADERCOMPILER_H_
#include "MGlobal.h"
#include "MShader.h"
#include "MRenderStructure.h"

class MORTY_CLASS MPreamble {
public:
	MPreamble();
	~MPreamble();

	bool IsValid() const;
	const char* GetText() const { return m_strText.c_str(); }

	const std::vector<std::string>& GetProcesses() { return m_vProcesses; }

	void AddDef(std::string def);

	void AddUndef(std::string undef);

protected:
	void FixLine(std::string& line);

private:
	std::vector<std::string> m_vProcesses;
	std::string m_strText;  // contents of preamble
};

class MORTY_CLASS MVulkanShaderCompiler
{
public:
    MVulkanShaderCompiler();
    ~MVulkanShaderCompiler();

public:

    bool Initialize();

    bool CompileShader(const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv);

	void ConvertMacro(const MShaderMacro& macro, MPreamble& preamble);

private:

};


#endif
