#include "MVulkanShaderCompiler.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <regex>

#include "Logger.h"
#include "SpvTools.h"
#include "GlslangToSpv.h"
#include "ShaderLang.h"

#include "MResource.h"
#include "MFileHelper.h"

#include "MEngine.h"
#include "MLogger.h"
#include "MVulkanDevice.h"

TBuiltInResource* InitResources()
{
	TBuiltInResource* pResult = new TBuiltInResource();
	TBuiltInResource& Resources =*pResult;

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

	return pResult;
}

class MVulkanIncluder : public glslang::TShader::Includer
{
public:

	MVulkanIncluder() :m_strLocalFolder("") {}

	void SetLocalFolder(const MString& strLocalFolder) { m_strLocalFolder = strLocalFolder + "/"; }


public:
	virtual IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		MString* pCode = new MString();
		if (!MFileHelper::ReadString(headerName, *pCode))
			return nullptr;
		
		glslang::TShader::Includer::IncludeResult* pResult = new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

		return pResult;
	}

	virtual IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		MString* pCode = new MString();
		if (!MFileHelper::ReadString(m_strLocalFolder + headerName, *pCode))
			return nullptr;

		glslang::TShader::Includer::IncludeResult* pResult = new glslang::TShader::Includer::IncludeResult(headerName, pCode->data(), pCode->length(), pCode);

		return pResult;
	}

	virtual void releaseInclude(IncludeResult* pResult) override
	{
		if (pResult)
		{
			MString* pCode = (MString*)pResult->userData;
			delete pCode;
		}
	}

private:

	MString m_strLocalFolder;
};



MVulkanShaderCompiler::MVulkanShaderCompiler(MVulkanDevice* pDevice)
	: m_pDefaultBuiltInResource(InitResources())
	, m_pDevice(pDevice)
{

}

MVulkanShaderCompiler::~MVulkanShaderCompiler()
{
	if (m_pDefaultBuiltInResource)
	{
		delete m_pDefaultBuiltInResource;
		m_pDefaultBuiltInResource = nullptr;
	}
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

bool MVulkanShaderCompiler::CompileShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
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

	if (true)
	{
		shader.setEnvInput(glslang::EShSourceHlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
		messages = (EShMessages)(messages | EShMsgReadHlsl);
	}
	else
	{
		shader.setEnvInput(glslang::EShSourceGlsl, eLanguageType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	}

	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	MVulkanIncluder includer;

	std::string strShaderDirectory;
	const size_t last_slash_idx = strShaderPath.rfind('/');
	if (std::string::npos != last_slash_idx)
	{
		strShaderDirectory = strShaderPath.substr(0, last_slash_idx);
	}
	includer.SetLocalFolder(strShaderDirectory);

	if (!shader.parse(m_pDefaultBuiltInResource, 100, false, messages, includer))
	{
		const char* a = shader.getInfoLog();
		const char* b = shader.getInfoDebugLog();
		m_pDevice->GetEngine()->GetLogger()->Error("%s\n\n\n%s", MString(shader.getInfoLog()).c_str(), MString(shader.getInfoDebugLog()).c_str());
		return false;
	}
	program.addShader(&shader);

	if (!program.link(messages))
		return false;


	if (program.getIntermediate(eLanguageType))
	{
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
#ifdef _DEBUG
// crash
//		 		spvOptions.generateDebugInfo = true;
//		 		spvOptions.stripDebugInfo = true;
#endif
		glslang::GlslangToSpv(*program.getIntermediate(eLanguageType), vSpirv, &logger, &spvOptions);

		if(!logger.getAllMessages().empty())
			m_pDevice->GetEngine()->GetLogger()->Information("%s", logger.getAllMessages().c_str());
//		{
// 			MString strOutputName = (strShaderPath + "v");
// 			const char* svBinaryName = strOutputName.c_str();
// 
// 			glslang::OutputSpvBin(spirv, svBinaryName);
// 		}

		return true;
	}

	return false;
}

void MVulkanShaderCompiler::ConvertMacro(const MShaderMacro& macro, MPreamble& preamble)
{
	for (const std::pair<MString, MString>& m : macro.s_vGlobalMacroParams)
		preamble.AddDef(m.first, m.second);
	
	for (const std::pair<MString, MString>& m : macro.m_vMortyMacroParams)
		preamble.AddDef(m.first, m.second);

	for (const std::pair<MString, MString>& m : macro.m_vMacroParams)
		preamble.AddDef(m.first, m.second);
}

void MVulkanShaderCompiler::GetVertexInputState(const spirv_cross::Compiler& compiler, MVertexShaderBuffer* pShaderBuffer)
{
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	//Vertex Input
	uint32_t unOffset = 0;
	for (const spirv_cross::Resource& res : shaderResources.stage_inputs)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		uint32_t unLocation = compiler.get_decoration(res.id, spv::Decoration::DecorationLocation);

		uint32_t unArraySize = type.array.empty() ? 1 : type.array[0];

		for (uint32_t nArrayIdx = 0; nArrayIdx < unArraySize; ++nArrayIdx)
		{
			VkVertexInputAttributeDescription attribute = {};
			attribute.binding = 0; // 它对应的是vertexBindingDescriptionCount，我们目前只有一个，这个值写死了是0
			attribute.location = unLocation;
			attribute.offset = unOffset;

			//	TODO fill attribute
			if (spirv_cross::SPIRType::BaseType::Float == type.basetype)
			{
				if (1 == type.vecsize)
					attribute.format = VK_FORMAT_R32_SFLOAT;
				else if (2 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32_SFLOAT;
				else if (3 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
				else if (4 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				else
					m_pDevice->GetEngine()->GetLogger()->Error("Error: vertex input find floatN ?");

				unOffset += sizeof(float) * type.vecsize;
			}
			else if (spirv_cross::SPIRType::BaseType::Int == type.basetype)
			{
				if (1 == type.vecsize)
					attribute.format = VK_FORMAT_R32_SINT;
				else if (2 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32_SINT;
				else if (3 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32_SINT;
				else if (4 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32A32_SINT;
				else
					m_pDevice->GetEngine()->GetLogger()->Error("Error: vertex input find intN ?");

				unOffset += sizeof(int) * type.vecsize;
			}

			attributeDescriptions.push_back(attribute);

			++unLocation;
		}
	}

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = unOffset;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	pShaderBuffer->m_vAttributeDescs = std::move(attributeDescriptions);
	pShaderBuffer->m_vBindingDescs = { bindingDescription };
}

void MVulkanShaderCompiler::GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer)
{
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

	for (const spirv_cross::Resource& res : shaderResources.uniform_buffers)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderConstantParam* pParam = new MShaderConstantParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		if (MGlobal::SHADER_PARAM_SET_MESH == pParam->unSet)
			pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		else
			pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		
		ConvertVariant(compiler, type, pParam->var);

		pShaderBuffer->m_vShaderSets[pParam->unSet].m_vParams.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_images)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderTextureParam* pParam = new MShaderTextureParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		if (type.image.dim == spv::Dim::DimCube)
			pParam->eType = METextureType::ETextureCube;

		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		pShaderBuffer->m_vShaderSets[pParam->unSet].m_vTextures.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_samplers)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderSampleParam* pParam = new MShaderSampleParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

		pShaderBuffer->m_vShaderSets[pParam->unSet].m_vSamples.push_back(pParam);
	}


	for (const spirv_cross::Resource& res : shaderResources.subpass_inputs)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderSubpasssInputParam* pParam = new MShaderSubpasssInputParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		pParam->m_VkDescriptorType =  VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		pShaderBuffer->m_vShaderSets[pParam->unSet].m_vTextures.push_back(pParam);
	}
}

void MVulkanShaderCompiler::ConvertVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant)
{
	MVariant tempVariant;

	switch (type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Struct:
	{
		tempVariant = MStruct();
		MStruct& srt = *tempVariant.GetStruct();

		for (uint32_t i = 0; i < type.member_types.size(); ++i)
		{
			const spirv_cross::TypeID& id = type.member_types[i];
			spirv_cross::SPIRType childType = compiler.get_type(id);

			spirv_cross::TypeID base_id = compiler.get_type(type.self).self;
			std::string strName = compiler.get_member_name(base_id, i);

			MVariant child;
			ConvertVariant(compiler, childType, child);
			srt.AppendMVariant(strName, child);
		}
		break;
	}
	default:
		ResetVariantType(type, tempVariant);
		break;
	}

	//数组判断
	if (type.array.empty())
	{
		variant.Move(tempVariant);
	}
	else
	{
		uint32_t unArraySize = type.array[0];

		variant = MVariantArray();
		MVariantArray& varArray = *variant.GetArray();

		for (uint32_t i = 0; i < unArraySize; ++i)
		{
			varArray.AppendMVariant(tempVariant);
		}
	}

}

bool MVulkanShaderCompiler::ResetVariantType(const spirv_cross::SPIRType& type, MVariant& variant)
{
	//todo support intN, boolN and matrixNxM


	switch (type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Struct:
		variant = MStruct();
		return true;

	case spirv_cross::SPIRType::BaseType::Boolean:
		variant = bool();
		return true;

	case spirv_cross::SPIRType::BaseType::Int:
		variant = int();
		return true;

	case spirv_cross::SPIRType::BaseType::UInt:
		variant = bool();		// TODO  spirv会把bool转成UInt，以后想办法解决吧。。
		return true;

	case spirv_cross::SPIRType::BaseType::Float:
	{
		switch (type.columns)
		{
		case 1:
		{
			switch (type.vecsize)
			{
			case 1:
				variant = float();
				return true;
			case 2:
				variant = Vector2();
				return true;
			case 3:
				variant = Vector3();
				return true;
			case 4:
				variant = Vector4();
				return true;
			default:
				break;
			}
			break;
		}
		case 3:
		{
			if (3 == type.vecsize)
			{
				variant = Matrix3();
				return true;
			}
			break;
		}
		case 4:
		{
			if (4 == type.vecsize)
			{
				variant = Matrix4();
				return true;
			}
			break;
		}

		default:
			break;
		}
		break;
	}
	}

	m_pDevice->GetEngine()->GetLogger()->Error("Can`t convert MVariant from spirv_cross::SPIRType. Unknow type");

	return false;
}

void MVulkanShaderCompiler::ReadShaderPath(const MString& strShaderPath)
{
	MString strShaderCode;
	MFileHelper::ReadString(strShaderPath, strShaderCode);

	std::regex r("\\s*#include\\s*\"(.*)\"");

	std::sregex_iterator pos(strShaderCode.cbegin(), strShaderCode.cend(), r);
	std::sregex_iterator end;

	for (; pos != end; ++pos)
	{
		m_pDevice->GetEngine()->GetLogger()->Information("include :%s", pos->str(1).c_str());
	}

	MString strFolder = MFileHelper::GetFileFolder(strShaderPath);

	MString strShaderFolder;



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