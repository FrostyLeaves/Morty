#include "MVulkanShaderReflector.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <regex>
#include <string>
#include <locale>

#include "Engine/MEngine.h"
#include "Utility/MLogger.h"
#include "Resource/MResource.h"
#include "Render/Vulkan/MVulkanDevice.h"

MVulkanShaderReflector::MVulkanShaderReflector(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
{

}

MVulkanShaderReflector::~MVulkanShaderReflector()
{
}

bool MVulkanShaderReflector::Initialize()
{
	return true;
}

void MVulkanShaderReflector::GetVertexInputState(const spirv_cross::Compiler& compiler, MVertexShaderBuffer* pShaderBuffer)
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


void MVulkanShaderReflector::GetComputeInputState(const spirv_cross::Compiler& compiler, MComputeShaderBuffer* pShaderBuffer)
{
	spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();
}

void MVulkanShaderReflector::GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer)
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

		std::string uav_name = compiler.get_name(res.id);
		if (uav_name.empty()) // cbuffer name from glslang.
		{
			uav_name = res.name;
		}

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

void MVulkanShaderReflector::ConvertVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant)
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

bool MVulkanShaderReflector::ResetVariantType(const spirv_cross::SPIRType& type, MVariant& variant)
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

#endif