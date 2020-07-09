#include "MVulkanShaderCompiler.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

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
#ifdef _DEBUG
		 		spvOptions.generateDebugInfo = true;
		 		spvOptions.stripDebugInfo = true;
#endif
		glslang::GlslangToSpv(*program.getIntermediate(eLanguageType), vSpirv, &logger, &spvOptions);

		MLogManager::GetInstance()->Information("%s", logger.getAllMessages().c_str());
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
					MLogManager::GetInstance()->Error("Error: vertex input find floatN ?");

				unOffset += sizeof(float) * type.vecsize;
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

		MShaderParam* pParam = new MShaderParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		

		ConvertVariant(compiler, type, pParam->var);

		pShaderBuffer->m_vShaderParamsTemplate.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_images)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderTextureParam* pParam = new MShaderTextureParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		pShaderBuffer->m_vTextureParamsTemplate.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_samplers)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		MShaderSampleParam* pParam = new MShaderSampleParam();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		pShaderBuffer->m_vSampleParamsTemplate.push_back(pParam);
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
		varArray.Resize(unArraySize);

		for(uint32_t i = 0; i < unArraySize; ++i)
			varArray[i] = tempVariant;
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

	MLogManager::GetInstance()->Error("Can`t convert MVariant from spirv_cross::SPIRType. Unknow type");

	return false;
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