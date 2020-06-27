#include "MVulkanShaderCompiler.h"

#include "Logger.h"
#include "SpvTools.h"
#include "GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "StandAlone/ResourceLimits.h"

#include "MResource.h"
#include "MFileHelper.h"

MVulkanShaderCompiler::MVulkanShaderCompiler()
{

}

MVulkanShaderCompiler::~MVulkanShaderCompiler()
{

}

bool MVulkanShaderCompiler::Initialize()
{
	glslang::InitializeProcess();
	glslang::InitializeProcess();  // also test reference counting of users
	glslang::InitializeProcess();  // also test reference counting of users
	glslang::FinalizeProcess();    // also test reference counting of users
	glslang::FinalizeProcess();    // also test reference counting of users

	return true;
}

bool MVulkanShaderCompiler::CompileShader(const MString& strShaderPath, const uint32_t& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{
	glslang::TProgram program;

	EShLanguage eLanguageType;
	MString strSuffix = MResource::GetSuffix(strShaderPath);
	if (strSuffix == "mvs")
		eLanguageType = EShLangVertex;
	else if (strSuffix == "mps")
		eLanguageType = EShLangFragment;
	else
		return false;

	glslang::TShader shader(eLanguageType);


	MString strShaderCode;
	MFileHelper::ReadString(strShaderPath, strShaderCode);

	const char* svShaderCode = strShaderCode.c_str();
	const char* svShaderPath = strShaderPath.c_str();
	shader.setStringsWithLengthsAndNames(&svShaderCode, NULL, &svShaderPath, 1);

	if (EShLangVertex == eLanguageType)
		shader.setEntryPoint("VS");
	else if (EShLangFragment == eLanguageType)
		shader.setEntryPoint("PS");

	MPreamble UserPreamble;
	ConvertMacro(macro, UserPreamble);
	if (UserPreamble.IsValid())
		shader.setPreamble(UserPreamble.GetText());
	shader.addProcesses(UserPreamble.GetProcesses());

	shader.setNanMinMaxClamp(false);

	// 	shader.setFlattenUniformArrays((Options & EOptionFlattenUniformArrays) != 0);
	// 	if (Options & EOptionHlslIoMapping)
	// 		shader.setHlslIoMapping(true);


	int ClientInputSemanticsVersion = 100;
	EShMessages messages = EShMsgDefault;

// 	if (true)
// 	{
// 		shader.setEnvInput(glslang::EShSourceHlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
// 		messages = (EShMessages)(messages | EShMsgReadHlsl);
// 	}
// 	else
	{
		shader.setEnvInput(glslang::EShSourceGlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	}

	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	TBuiltInResource Resources = glslang::DefaultTBuiltInResource;


	if (!shader.parse(&Resources, 100, false, messages))
	{
		MLogManager::GetInstance()->Error("%s\n\n\n%s", shader.getInfoLog(), shader.getInfoDebugLog());
		return false;
	}
	program.addShader(&shader);

	if (!program.link(messages))
		return false;


	if (program.getIntermediate(eLanguageType))
	{
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		// #ifdef _DEBUG
		// 		spvOptions.generateDebugInfo = true;
		// 		spvOptions.stripDebugInfo = true;
		// #endif
		glslang::GlslangToSpv(*program.getIntermediate(eLanguageType), vSpirv, &logger, &spvOptions);

		MLogManager::GetInstance()->Information("%s", logger.getAllMessages().c_str());
//		{
// 			MString strOutputName = (strShaderPath + "v");
// 			const char* svBinaryName = strOutputName.c_str();
// 
// 			glslang::OutputSpvBin(spirv, svBinaryName);
// 		}
	}
}

void MVulkanShaderCompiler::ConvertMacro(const MShaderMacro& macro, MPreamble& preamble)
{

}

MPreamble::MPreamble()
{

}

MPreamble::~MPreamble()
{

}

bool MPreamble::IsValid() const
{
	return m_strText.size() > 0;
}

void MPreamble::AddDef(std::string def)
{
	m_strText.append("#define ");
	FixLine(def);

	m_vProcesses.push_back("define-macro ");
	m_vProcesses.back().append(def);

	// The first "=" needs to turn into a space
	const size_t equal = def.find_first_of("=");
	if (equal != def.npos)
		def[equal] = ' ';

	m_strText.append(def);
	m_strText.append("\n");
}

void MPreamble::AddUndef(std::string undef)
{
	m_strText.append("#undef ");
	FixLine(undef);

	m_vProcesses.push_back("undef-macro ");
	m_vProcesses.back().append(undef);

	m_strText.append(undef);
	m_strText.append("\n");
}

void MPreamble::FixLine(std::string& line)
{
	// Can't go past a newline in the line
	const size_t end = line.find_first_of("\n");
	if (end != line.npos)
		line = line.substr(0, end);
}
