#include "Render/Vulkan/MVulkanShaderCompiler.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <regex>
#include <string>
#include <locale>
#include <codecvt>

#include "dxcapi.h"

#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"

#include "Engine/MEngine.h"
#include "Utility/MLogger.h"
#include "Render/Vulkan/MVulkanDevice.h"

struct MVulkanIncludeHandler : public IDxcIncludeHandler
{
	MVulkanIncludeHandler(IDxcUtils* pUtils, IDxcIncludeHandler* pDefaultIncludeHandler)
		: m_pUtils(pUtils)
		, m_pIncludeHandler(pDefaultIncludeHandler)
		, m_vSearchPath()
	{
		
	}

	HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
	{
		IDxcBlobEncoding* pEncoding = nullptr;
		HRESULT hr = m_pUtils->LoadFile(pFilename, nullptr, &pEncoding);
		if (SUCCEEDED(hr))
		{
			*ppIncludeSource = pEncoding;
		}
		else
		{
			*ppIncludeSource = nullptr;
		}
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
};

MVulkanShaderCompiler::MVulkanShaderCompiler(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

MVulkanShaderCompiler::~MVulkanShaderCompiler()
{
}

bool MVulkanShaderCompiler::Initialize()
{
	return true;
}

bool MVulkanShaderCompiler::CompileShader(const MString& strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{
	return CompileHlslShader(strShaderPath, eShaderType, macro, vSpirv);
}

bool MVulkanShaderCompiler::CompileHlslShader(const MString& _strShaderPath, const MEShaderType& eShaderType, const MShaderMacro& macro, std::vector<uint32_t>& vSpirv)
{
	MString strShaderPath = MFileHelper::FormatPath(_strShaderPath);
	//MStringHelper::Replace(strShaderPath, "/", "\\");

	IDxcUtils* pUtils = nullptr;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
	
	 
	IDxcCompiler3* pCompiler = nullptr;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

	// Create default include handler. (You can create your own...)
	//
	IDxcIncludeHandler* pDefaultIncludeHandler = nullptr;
	pUtils->CreateDefaultIncludeHandler(&pDefaultIncludeHandler);

	std::shared_ptr<MVulkanIncludeHandler> pIncludeHandler = std::make_shared<MVulkanIncludeHandler>(pUtils, pDefaultIncludeHandler);

	std::wstring_convert<std::codecvt_utf8<wchar_t>> stow;

	std::wstring wstrShaderPath = stow.from_bytes(strShaderPath);

	std::vector<std::wstring> vCompArgs;
	vCompArgs.push_back(wstrShaderPath);
	
	//Macro
	MPreamble UserPreamble;
	ConvertMacroDXC(macro, UserPreamble);
	if (UserPreamble.IsValid())
	{
		for (const std::pair<MString, MString>& m : macro.s_vGlobalMacroParams)
		{
			if(m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first + "=" + m.second));
		}

		for (const std::pair<MString, MString>& m : macro.m_vMortyMacroParams)
		{
			if (m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first + "=" + m.second));
		}

		for (const std::pair<MString, MString>& m : macro.m_vMacroParams)
		{
			if (m.second.empty())
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first));
			else
				vCompArgs.push_back(std::wstring(L"-D ") + stow.from_bytes(m.first + "=" + m.second));
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
	else
		MORTY_ASSERT(false);

	//vCompArgs.push_back(L"-enable-templates");

	vCompArgs.push_back(L"-spirv");
	vCompArgs.push_back(L"-fspv-extension=SPV_NV_ray_tracing");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_multiview");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_shader_draw_parameters");
	vCompArgs.push_back(L"-fspv-extension=SPV_EXT_descriptor_indexing");

#ifdef MORTY_DEBUG
	vCompArgs.push_back(L"-Od");
	vCompArgs.push_back(L"-Zi");
	//vCompArgs.push_back(L"-Qembed_debug");
	//vCompArgs.push_back(L"-fspv-target-env=vulkan1.2");
	vCompArgs.push_back(L"-fspv-extension=SPV_KHR_non_semantic_info");
	vCompArgs.push_back(L"-fspv-debug=vulkan-with-source");
	//vCompArgs.push_back(L"-fspv-debug=rich-with-source");
#else
	vCompArgs.push_back(L"-Zs");
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
		m_pDevice->GetEngine()->GetLogger()->Information("hlsl compile output: \n%s", pErrors->GetStringPointer());
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

void MVulkanShaderCompiler::ConvertMacro(const MShaderMacro& macro, MPreamble& preamble)
{
	for (const std::pair<MString, MString>& m : macro.s_vGlobalMacroParams)
		preamble.AddDef(m.first, m.second);
	
	for (const std::pair<MString, MString>& m : macro.m_vMortyMacroParams)
		preamble.AddDef(m.first, m.second);

	for (const std::pair<MString, MString>& m : macro.m_vMacroParams)
		preamble.AddDef(m.first, m.second);
}

void MVulkanShaderCompiler::ConvertMacroDXC(const MShaderMacro& macro, MPreamble& preamble)
{
	for (const std::pair<MString, MString>& m : macro.s_vGlobalMacroParams)
		preamble.AddDef2(m.first, m.second);

	for (const std::pair<MString, MString>& m : macro.m_vMortyMacroParams)
		preamble.AddDef2(m.first, m.second);

	for (const std::pair<MString, MString>& m : macro.m_vMacroParams)
		preamble.AddDef2(m.first, m.second);
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
			attribute.binding = 0; // ����Ӧ����vertexBindingDescriptionCount������Ŀǰֻ��һ�������ֵд������0
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
			else if (spirv_cross::SPIRType::BaseType::UInt == type.basetype)
			{
				if (1 == type.vecsize)
					attribute.format = VK_FORMAT_R32_UINT;
				else if (2 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32_UINT;
				else if (3 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32_UINT;
				else if (4 == type.vecsize)
					attribute.format = VK_FORMAT_R32G32B32A32_UINT;
				else
					m_pDevice->GetEngine()->GetLogger()->Error("Error: vertex input find intN ?");

				unOffset += sizeof(int) * type.vecsize;
			}
			else
			{
				MORTY_ASSERT(false);
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


void MVulkanShaderCompiler::GetComputeInputState(const spirv_cross::Compiler& compiler, MComputeShaderBuffer* pShaderBuffer)
{
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();
}

void MVulkanShaderCompiler::GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer)
{
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

	for (const spirv_cross::Resource& res : shaderResources.storage_buffers)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(res.type_id);

		std::shared_ptr<MShaderStorageParam> pParam = std::make_shared<MShaderStorageParam>();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);

		const std::string& uav_name = compiler.get_name(res.id);
		pParam->strName = uav_name;

		spirv_cross::Bitset buffer_flags = compiler.get_buffer_block_flags(res.id);
		pParam->bWritable = !buffer_flags.get(spv::DecorationNonWritable);


		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

		pShaderBuffer->m_vShaderSets[pParam->unSet]->m_vStorages.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.uniform_buffers)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		std::shared_ptr<MShaderConstantParam> pParam = std::make_shared<MShaderConstantParam>();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
//		MStringHelper::Replace(pParam->strName, "type.", "");

		const std::string& uav_name = compiler.get_name(res.id);
		pParam->strName = uav_name;

		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		
		ConvertVariant(compiler, type, pParam->var);

		pShaderBuffer->m_vShaderSets[pParam->unSet]->m_vParams.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_images)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		std::shared_ptr<MShaderTextureParam> pParam = std::make_shared<MShaderTextureParam>();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		if (type.image.dim == spv::Dim::DimCube)
		{
			pParam->eType = METextureType::ETextureCube;
		}

		else if (type.image.arrayed)
		{
			pParam->eType = METextureType::ETexture2DArray;
		}

		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		pShaderBuffer->m_vShaderSets[pParam->unSet]->m_vTextures.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.separate_samplers)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		std::shared_ptr<MShaderSampleParam> pParam = std::make_shared<MShaderSampleParam>();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		if (!pParam->strName.empty())
		{
			if ('L' == pParam->strName[0])
				pParam->eSamplerType = MESamplerType::ELinear;
			else
				pParam->eSamplerType = MESamplerType::ENearest;
		}

		pParam->m_VkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

		pShaderBuffer->m_vShaderSets[pParam->unSet]->m_vSamples.push_back(pParam);
	}

	for (const spirv_cross::Resource& res : shaderResources.subpass_inputs)
	{
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);

		std::shared_ptr<MShaderSubpasssInputParam> pParam = std::make_shared<MShaderSubpasssInputParam>();
		pParam->unSet = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
		pParam->strName = res.name;

		pParam->m_VkDescriptorType =  VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

		pShaderBuffer->m_vShaderSets[pParam->unSet]->m_vTextures.push_back(pParam);
	}
}

void MVulkanShaderCompiler::ConvertVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant)
{
	MVariant tempVariant;

	switch (type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Struct:
	{
		MVariantStruct srt;
		MVariantStructBuilder builder(srt);
		for (uint32_t i = 0; i < type.member_types.size(); ++i)
		{
			const spirv_cross::TypeID& id = type.member_types[i];
			spirv_cross::SPIRType childType = compiler.get_type(id);

			spirv_cross::TypeID base_id = compiler.get_type(type.self).self;
			std::string strName = compiler.get_member_name(base_id, i);

			MVariant child;
			ConvertVariant(compiler, childType, child);
			builder.AppendVariant(strName, child);
		}
		builder.Finish();
		tempVariant = std::move(MVariant(srt));
		break;
	}
	default:
		ResetVariantType(type, tempVariant);
		break;
	}

	if (type.array.empty())
	{
		variant = std::move(tempVariant);
	}
	else
	{
		uint32_t unArraySize = type.array[0];

		MVariantArray varArray;
		MVariantArrayBuilder builder(varArray);

		for (uint32_t i = 0; i < unArraySize; ++i)
		{
			builder.AppendVariant(tempVariant);
		}
		builder.Finish();
		variant = std::move(MVariant(varArray));
	}

}

bool MVulkanShaderCompiler::ResetVariantType(const spirv_cross::SPIRType& type, MVariant& variant)
{
	//todo support intN, boolN and matrixNxM

	switch (type.basetype)
	{
	case spirv_cross::SPIRType::BaseType::Struct:
		MORTY_ASSERT(false);
		return true;

	case spirv_cross::SPIRType::BaseType::Boolean:
		variant = MVariant(bool());
		return true;

	case spirv_cross::SPIRType::BaseType::Int:
		variant = MVariant(int());
		return true;

	case spirv_cross::SPIRType::BaseType::UInt:
		variant = MVariant(bool());		// TODO  spirv���boolת��UInt���Ժ���취����ɡ���
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
				variant = MVariant(float());
				return true;
			case 2:
				variant = MVariant(Vector2());
				return true;
			case 3:
				variant = MVariant(Vector3());
				return true;
			case 4:
				variant = MVariant(Vector4());
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
				variant = MVariant(Matrix3());
				return true;
			}
			break;
		}
		case 4:
		{
			if (4 == type.vecsize)
			{
				variant = MVariant(Matrix4());
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

void MPreamble::AddDef2(const MString& strName, const MString& strValue)
{
	if (strValue.empty())
	{
		m_strText.append(strName);
		m_strText.append(" ");
	}
	else
	{
		m_strText.append(strName + "=" + strValue + " ");
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