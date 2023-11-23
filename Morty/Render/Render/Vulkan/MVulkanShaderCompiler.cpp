#include "Render/Vulkan/MVulkanShaderCompiler.h"
#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <regex>
#include <string>
#include <locale>
#include <codecvt>

#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "Utility/MLogger.h"
#include "Render/Vulkan/MVulkanDevice.h"


#ifdef MORTY_SHADER_COMPILER_DXC
#include "dxcapi.h"
#else

#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"

#endif


#ifdef MORTY_SHADER_COMPILER_DXC
struct MVulkanIncludeHandler : public IDxcIncludeHandler
{
	MVulkanIncludeHandler(IDxcUtils* pUtils, IDxcIncludeHandler* pDefaultIncludeHandler)
		: m_pUtils(pUtils)
		, m_pIncludeHandler(pDefaultIncludeHandler)
	{

	}

	virtual ~MVulkanIncludeHandler() = default;

	void SetLocalPath(const std::wstring& strShaderDir)
	{
		m_strShaderDir = strShaderDir;
	}

	void SetSystemSearchPath (const std::vector<std::wstring>& paths)
	{
		m_vSearchPath = paths;
	}

	HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
	{
		IDxcBlobEncoding* pEncoding = nullptr;

		auto pRelativeFileName = pFilename + m_strShaderDir.size();

		HRESULT hr = MGlobal::M_INVALID_UINDEX;
		for (const auto& searchPath : m_vSearchPath )
		{
			auto fullPath = searchPath + pRelativeFileName;
			hr = m_pUtils->LoadFile(fullPath.c_str(), nullptr, &pEncoding);
			if (SUCCEEDED(hr))
			{
				*ppIncludeSource = pEncoding;
				return hr;
			}
		}

		*ppIncludeSource = nullptr;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
	{
		return m_pIncludeHandler->QueryInterface(riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
	ULONG STDMETHODCALLTYPE Release(void) override { return 0; }
private:
	IDxcUtils* m_pUtils;
	IDxcIncludeHandler* m_pIncludeHandler;
	std::vector<std::wstring> m_vSearchPath;
	std::wstring m_strShaderDir;
};

#else
class MVulkanIncludeHandler : public glslang::TShader::Includer
{
public:
	MVulkanIncludeHandler() = default;

	void SetSystemSearchPath(const std::vector<std::string>& paths)
	{
		m_vSearchPath = paths;
	}

	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		MORTY_UNUSED(includerName);
		MORTY_UNUSED(inclusionDepth);

		MString* pCode = new MString();
		if (!MFileHelper::ReadString(headerName, *pCode))
			return nullptr;

		glslang::TShader::Includer::IncludeResult* pResult = new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

		return pResult;
	}

	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		MORTY_UNUSED(includerName);
		MORTY_UNUSED(inclusionDepth);

		MString* pCode = new MString();
        for (auto strLocalFolder : m_vSearchPath)
        {
            if (MFileHelper::ReadString(strLocalFolder + headerName, *pCode))
			{
                break;
			}
        }
        
        if (pCode->size() == 0)
        {
            return nullptr;
        }
        
		glslang::TShader::Includer::IncludeResult* pResult = new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

		return pResult;
	}

	void releaseInclude(IncludeResult* pResult) override
	{
		if (pResult)
		{
			MString* pCode = (MString*)pResult->userData;
			delete pCode;
		}
	}

private:

	std::vector<std::string> m_vSearchPath;
};

#endif

MVulkanShaderCompiler::MVulkanShaderCompiler(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{
#ifdef MORTY_SHADER_COMPILER_DXC
#else
	glslang::InitializeProcess();
#endif
}

MVulkanShaderCompiler::~MVulkanShaderCompiler()
{
#ifdef MORTY_SHADER_COMPILER_DXC
#else
	glslang::FinalizeProcess();    // also test reference counting of users
#endif
}

bool MVulkanShaderCompiler::CompileShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{
	return CompileHlslShader(strShaderPath, eShaderType, macro, vSpirv);
}

#ifdef MORTY_SHADER_COMPILER_DXC
bool MVulkanShaderCompiler::CompileHlslShader(const MString& _strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{
	MString strShaderPath = MFileHelper::FormatPath(_strShaderPath);
	auto strShaderLocalDir = MStringUtil::ConvertToWString(MFileHelper::GetFileFolder(strShaderPath) + "/");

	IDxcUtils* pUtils = nullptr;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
	
	 
	IDxcCompiler3* pCompiler = nullptr;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

	// Create default include handler. (You can create your own...)
	//
	IDxcIncludeHandler* pDefaultIncludeHandler = nullptr;
	pUtils->CreateDefaultIncludeHandler(&pDefaultIncludeHandler);


	std::shared_ptr<MVulkanIncludeHandler> pIncludeHandler = std::make_shared<MVulkanIncludeHandler>(pUtils, pDefaultIncludeHandler);
	pIncludeHandler->SetSystemSearchPath({
		strShaderLocalDir,
		MStringUtil::ConvertToWString( MORTY_RESOURCE_PATH ) + L"/Shader/"
	});
	pIncludeHandler->SetLocalPath(strShaderLocalDir);


	std::wstring wstrShaderPath = MStringUtil::ConvertToWString(strShaderPath);

	std::vector<std::wstring> vCompArgs;
	vCompArgs.push_back(wstrShaderPath);
	
	//Macro
	MPreamble UserPreamble;
	ConvertMacro(macro, UserPreamble);
	if (UserPreamble.IsValid())
	{
		for (const auto& m : macro.s_vGlobalMacroParams)
		{
			if(m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second));
		}

		for (const auto& m : macro.m_vMortyMacroParams)
		{
			if (m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second));
		}

		for (const auto& m : macro.m_vMacroParams)
		{
			if (m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString()));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + MStringUtil::ConvertToWString(m.first.ToString() + "=" + m.second));
		}
	}

	if (MEShaderType::EVertex == eShaderType)
	{
		vCompArgs.push_back(L"-E VS_MAIN");
		vCompArgs.push_back(L"-T vs_6_1");
	}
	else if (MEShaderType::EPixel == eShaderType)
	{
		vCompArgs.push_back(L"-E PS_MAIN");
		vCompArgs.push_back(L"-T ps_6_1");
	}
	else if (MEShaderType::ECompute == eShaderType)
	{
		vCompArgs.push_back(L"-E CS_MAIN");
		vCompArgs.push_back(L"-T cs_6_1");
	}
	else if (MEShaderType::EGeometry == eShaderType)
	{
		vCompArgs.push_back(L"-E GS_MAIN");
		vCompArgs.push_back(L"-T gs_6_1");
	}
	else
	{
		MORTY_ASSERT(false);
	}

	//vCompArgs.push_back(L"-enable-templates");

	vCompArgs.push_back(L"-spirv");
	vCompArgs.push_back(L"-fspv-target-env=vulkan1.2");
	vCompArgs.push_back(L"-fspv-extension=SPV_NV_ray_tracing");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_multiview");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_shader_draw_parameters");
	vCompArgs.push_back(L"-fspv-extension=SPV_EXT_descriptor_indexing");


//	vCompArgs.push_back(L"-fspv-reflect");
//	vCompArgs.push_back(L"-fspv-extension=SPV_GOOGLE_user_type");
//	vCompArgs.push_back(L"-fspv-extension=SPV_GOOGLE_hlsl_functionality1");

#if MORTY_DEBUG
	vCompArgs.push_back(L"-Od");
	vCompArgs.push_back(L"-Zi");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_non_semantic_info");
	vCompArgs.push_back(L"-fspv-debug=vulkan-with-source");
	//vCompArgs.push_back(L"-fspv-debug=rich-with-source");
#else
	vCompArgs.push_back(L"-Zs");
	vCompArgs.push_back(L"-Oconfig="
		"--ccp,"
		"--cfg-cleanup,"
		"--convert-local-access-chains,"
		"--copy-propagate-arrays,"
		"--eliminate-dead-branches,"
//		"--eliminate-dead-code-aggressive,"		//it will remove unused (binding,set)
		"--eliminate-dead-functions,"
		"--eliminate-local-multi-store,"
		"--eliminate-local-single-block,"
		"--eliminate-local-single-store,"
		"--flatten-decorations,"
		"--if-conversion,"
		"--inline-entry-points-exhaustive,"
		"--local-redundancy-elimination,"
    );
#endif


	//
	// Open source file.  
	//
	IDxcBlobEncoding* pSource = nullptr;
	pUtils->LoadFile(wstrShaderPath.c_str(), nullptr, &pSource);

	DxcBuffer Source;
	Source.Ptr = pSource->GetBufferPointer();
	Source.Size = pSource->GetBufferSize();
	Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

	LPCWSTR* pszArgs = new LPCWSTR[vCompArgs.size()];

	for(size_t i = 0; i < vCompArgs.size(); ++i)
	{
		pszArgs[i] = vCompArgs[i].c_str();
	};

	IDxcResult* pResults = nullptr;
	HRESULT hrCompile = pCompiler->Compile(&Source, pszArgs, vCompArgs.size(), pIncludeHandler.get(), IID_PPV_ARGS(&pResults));

	delete[] pszArgs;
	
	IDxcBlobUtf8* pErrors = nullptr;
	pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	if (pErrors != nullptr && pErrors->GetStringLength() != 0)
	{
		MORTY_ASSERT(pErrors);
		m_pDevice->GetEngine()->GetLogger()->Information("hlsl compile output: \n{}", pErrors->GetStringPointer());
	}

	HRESULT hrStatus;
	pResults->GetStatus(&hrStatus);
	if (FAILED(hrCompile))
	{
		m_pDevice->GetEngine()->GetLogger()->Error("Compilation Failed.");
		return false;
	}

	IDxcBlob* pShader = nullptr;
	IDxcBlobUtf16* pShaderNameTemp = nullptr;
	pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderNameTemp);
	std::shared_ptr<IDxcBlobUtf16> pShaderName(pShaderNameTemp);

	if (!pShader)
		return false;

	vSpirv.resize(pShader->GetBufferSize() / sizeof(uint32_t));
	memcpy(vSpirv.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());


	return true;

}
#else
TBuiltInResource* GlslangDefaultResources()
{
	static TBuiltInResource Resources;

	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.maxMeshOutputVerticesNV = 256;
	Resources.maxMeshOutputPrimitivesNV = 512;
	Resources.maxMeshWorkGroupSizeX_NV = 32;
	Resources.maxMeshWorkGroupSizeY_NV = 1;
	Resources.maxMeshWorkGroupSizeZ_NV = 1;
	Resources.maxTaskWorkGroupSizeX_NV = 32;
	Resources.maxTaskWorkGroupSizeY_NV = 1;
	Resources.maxTaskWorkGroupSizeZ_NV = 1;
	Resources.maxMeshViewCountNV = 4;

	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;

	return &Resources;
}

bool MVulkanShaderCompiler::CompileHlslShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{

	static std::map<MEShaderType, EShLanguage> ShaderTypeTable = {
		{ MEShaderType::EVertex, EShLangVertex},
		{ MEShaderType::EPixel, EShLangFragment},
		{ MEShaderType::ECompute, EShLangCompute},
		{ MEShaderType::EGeometry, EShLangGeometry},
	};

	static std::map<MEShaderType, MString> ShaderEntryTable = {
		{ MEShaderType::EVertex, "VS_MAIN"},
		{ MEShaderType::EPixel, "PS_MAIN"},
		{ MEShaderType::ECompute, "CS_MAIN"},
		{ MEShaderType::EGeometry, "GS_MAIN"},
	};

	if (ShaderTypeTable.find(eShaderType) == ShaderTypeTable.end())
	{
		MORTY_ASSERT(false);
		return false;
	}

	if (ShaderEntryTable.find(eShaderType) == ShaderEntryTable.end())
	{
		MORTY_ASSERT(false);
		return false;
	}


	EShLanguage eLanguageType = ShaderTypeTable[eShaderType];
	glslang::TShader shader(eLanguageType);

	MString strShaderCode;
	MFileHelper::ReadString(strShaderPath, strShaderCode);


	const char* svShaderCode = strShaderCode.c_str();
	const char* svShaderPath = strShaderPath.c_str();
	shader.setStringsWithLengthsAndNames(&svShaderCode, NULL, &svShaderPath, 1);
	shader.setEntryPoint(ShaderEntryTable[eShaderType].c_str());

	MPreamble UserPreamble;
	ConvertMacro(macro, UserPreamble);

	if (UserPreamble.IsValid())
	{
		shader.setPreamble(UserPreamble.GetText());
	}
	shader.addProcesses(UserPreamble.GetProcesses());

	shader.setNanMinMaxClamp(false);

	// 	shader.setFlattenUniformArrays((Options & EOptionFlattenUniformArrays) != 0);
	// 	if (Options & EOptionHlslIoMapping)
	// 		shader.setHlslIoMapping(true);
	//shader.setEnvTargetHlslFunctionality1();


	int ClientInputSemanticsVersion = 120;
	EShMessages messages = EShMsgDefault;
	
	shader.setEnvInput(glslang::EShSourceHlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	messages = EShMessages(messages | EShMsgReadHlsl);

	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_2);

	MVulkanIncludeHandler includer;

	std::string strShaderDirectory;
	const size_t last_slash_idx = strShaderPath.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
		strShaderDirectory = strShaderPath.substr(0, last_slash_idx);
	}

	includer.SetSystemSearchPath({
		MORTY_RESOURCE_PATH "/Shader/",
		strShaderDirectory + "/"
	});
	
	if (!shader.parse(GlslangDefaultResources(), 120, false, messages, includer))
	{
		m_pDevice->GetEngine()->GetLogger()->Error("{}\n\n\n{}", MString(shader.getInfoLog()).c_str(), MString(shader.getInfoDebugLog()).c_str());
		return false;
	}

	glslang::TProgram program;

	program.addShader(&shader);

	if (!program.link(messages))
	{
		return false;
	}

	if (program.getIntermediate(eLanguageType))
	{
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
#ifdef MORTY_DEBUG
		// crash
		//		 		spvOptions.generateDebugInfo = true;
		//		 		spvOptions.stripDebugInfo = true;
#endif
		glslang::GlslangToSpv(*program.getIntermediate(eLanguageType), vSpirv, &logger, &spvOptions);

		if (!logger.getAllMessages().empty())
		{
			m_pDevice->GetEngine()->GetLogger()->Information("{}", logger.getAllMessages().c_str());
		}

		return true;
	}

	return false;
}
#endif

void MVulkanShaderCompiler::ConvertMacro(const MShaderMacro& macro, MPreamble& preamble)
{
	for (const auto& m : macro.s_vGlobalMacroParams)
		preamble.AddDef(m.first.ToString(), m.second);
	
	for (const auto& m : macro.m_vMortyMacroParams)
		preamble.AddDef(m.first.ToString(), m.second);

	for (const auto& m : macro.m_vMacroParams)
		preamble.AddDef(m.first.ToString(), m.second);
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

void MPreamble::AddDef(const MString& strName, const MString& strValue)
{
#ifdef MORTY_SHADER_COMPILER_DXC
	if (strValue.empty())
	{
		m_strText.append(strName);
		m_strText.append(" ");
	}
	else
	{
		m_strText.append(strName + "=" + strValue + " ");
	}
#else
	m_strText.append("#define ");

	m_vProcesses.push_back("define-macro ");
	if (strValue.empty())
	{
		m_vProcesses.back().append(strName);
		m_strText.append(strName);
		m_strText.append("\n");
	}
	else
	{
		m_vProcesses.back().append(strName + "=" + strValue);
		m_strText.append(strName + " " + strValue + "\n");
	}
#endif
}

void MPreamble::AddUndef(std::string undef)
{
	m_strText.append("#undef ");

	m_vProcesses.push_back("undef-macro ");
	m_vProcesses.back().append(undef);

	m_strText.append(undef);
	m_strText.append("\n");
}


#endif
