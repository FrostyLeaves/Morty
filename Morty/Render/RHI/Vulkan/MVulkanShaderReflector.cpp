#include "MVulkanShaderReflector.h"


#if RENDER_GRAPHICS == MORTY_VULKAN

#include <locale>
#include <regex>
#include <string>

#include "Engine/MEngine.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Resource/MResource.h"
#include "Utility/MLogger.h"

using namespace morty;

MVulkanShaderReflector::MVulkanShaderReflector(MVulkanDevice* pDevice)
    : m_device(pDevice)
{}

MVulkanShaderReflector::~MVulkanShaderReflector() {}

bool MVulkanShaderReflector::Initialize() { return true; }


enum class ShaderReflectorType
{
    Undefine = 0,
    U32,
};

bool CheckIgnoreVertexInput(const MString name)
{
    MORTY_UNUSED(name);
    //if (name == "ViewIndex")
    //{
    //	return true;
    //}

    return false;
}

ShaderReflectorType GetReflectorTypeFromName(const std::string& name)
{
    if (name.size() < 3) { return ShaderReflectorType::Undefine; }

    const size_t offset = name.rfind('.');

    if (name.compare(offset, 3, "u32")) { return ShaderReflectorType::U32; }

    return ShaderReflectorType::Undefine;
}

std::tuple<VkFormat, uint32_t>
MVulkanShaderReflector::GetVertexInputDescription(const std::string& name, spirv_cross::SPIRType type) const
{
    if (spirv_cross::SPIRType::BaseType::Float == type.basetype)
    {
        static std::array<std::tuple<VkFormat, uint32_t>, 4> sFormatMapping = {
                std::make_tuple(VK_FORMAT_R32_SFLOAT, 4),
                {VK_FORMAT_R32G32_SFLOAT, 8},
                {VK_FORMAT_R32G32B32_SFLOAT, 12},
                {VK_FORMAT_R32G32B32A32_SFLOAT, 16}};

        if (4 == type.vecsize)
        {
            const ShaderReflectorType eReflectorType = GetReflectorTypeFromName(name);
            if (eReflectorType == ShaderReflectorType::U32) { return {VK_FORMAT_R8G8B8A8_UNORM, 4}; }
        }

        if (type.vecsize >= sFormatMapping.size())
        {
            MORTY_ASSERT(type.vecsize <= sFormatMapping.size());
            m_device->GetEngine()->GetLogger()->Error("Error: vertex input find floatN ?");
            return {VK_FORMAT_UNDEFINED, 0};
        }

        return sFormatMapping[type.vecsize - 1];
    }
    if (spirv_cross::SPIRType::BaseType::Int == type.basetype)
    {
        static std::array<std::tuple<VkFormat, uint32_t>, 4> sFormatMapping = {
                std::make_tuple(VK_FORMAT_R32_SINT, 4),
                {VK_FORMAT_R32G32_SINT, 8},
                {VK_FORMAT_R32G32B32_SINT, 12},
                {VK_FORMAT_R32G32B32A32_SINT, 16}};
        if (type.vecsize >= sFormatMapping.size())
        {
            MORTY_ASSERT(type.vecsize <= sFormatMapping.size());
            m_device->GetEngine()->GetLogger()->Error("Error: vertex input find intN ?");
            return {VK_FORMAT_UNDEFINED, 0};
        }

        return sFormatMapping[type.vecsize - 1];
    }
    if (spirv_cross::SPIRType::BaseType::UInt == type.basetype)
    {
        static std::array<std::tuple<VkFormat, uint32_t>, 4> sFormatMapping = {
                std::make_tuple(VK_FORMAT_R32_UINT, 4),
                {VK_FORMAT_R32G32_UINT, 8},
                {VK_FORMAT_R32G32B32_UINT, 12},
                {VK_FORMAT_R32G32B32A32_UINT, 16}};
        if (type.vecsize >= sFormatMapping.size())
        {
            MORTY_ASSERT(type.vecsize <= sFormatMapping.size());
            m_device->GetEngine()->GetLogger()->Error("Error: vertex input find uintN ?");
            return {VK_FORMAT_UNDEFINED, 0};
        }

        return sFormatMapping[type.vecsize - 1];
    }

    MORTY_ASSERT(false);
    return {VK_FORMAT_UNDEFINED, 0};
}

void MVulkanShaderReflector::GetVertexInputState(
        const spirv_cross::Compiler& compiler,
        MVertexShaderBuffer*         pShaderBuffer
) const
{
    spirv_cross::ShaderResources                   shaderResources = compiler.get_shader_resources();

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    uint32_t                                       nInputStride = 0;

    for (const spirv_cross::Resource& res: shaderResources.stage_inputs)
    {
        const MString& name = compiler.get_name(res.id);

        if (CheckIgnoreVertexInput(name)) { continue; }

        spirv_cross::SPIRType type          = compiler.get_type(res.type_id);
        const auto [format, nInputTypeSize] = GetVertexInputDescription(name, type);

        const uint32_t unLocation  = compiler.get_decoration(res.id, spv::Decoration::DecorationLocation);
        const uint32_t unArraySize = type.array.empty() ? 1 : type.array[0];
        for (uint32_t nArrayIdx = 0; nArrayIdx < unArraySize; ++nArrayIdx)
        {
            VkVertexInputAttributeDescription attribute = {};
            attribute.location                          = unLocation + nArrayIdx;
            attribute.binding                           = 0;
            attribute.format                            = format;
            attribute.offset                            = nInputStride + nInputTypeSize * nArrayIdx;

            attributeDescriptions.push_back(attribute);
        }

        nInputStride += nInputTypeSize * unArraySize;
    }

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding   = 0;
    bindingDescription.stride    = nInputStride;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    pShaderBuffer->m_attributeDescs = std::move(attributeDescriptions);
    pShaderBuffer->m_bindingDescs   = {bindingDescription};
}

void MVulkanShaderReflector::GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer)
{
    spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();

    for (const spirv_cross::Resource& res: shaderResources.storage_buffers)
    {
        //const spirv_cross::SPIRType& type = compiler.get_type(res.type_id);

        std::shared_ptr<MShaderStorageParam> pParam = std::make_shared<MShaderStorageParam>();
        pParam->unSet                               = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);

        const std::string& uav_name = compiler.get_name(res.id);
        pParam->strName             = MStringId(uav_name.c_str());

        spirv_cross::Bitset buffer_flags = compiler.get_buffer_block_flags(res.id);
        pParam->bWritable                = !buffer_flags.get(spv::DecorationNonWritable);


        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_storages.push_back(pParam);
    }

    for (const spirv_cross::Resource& res: shaderResources.uniform_buffers)
    {
        spirv_cross::SPIRType                 type = compiler.get_type(res.type_id);

        std::shared_ptr<MShaderConstantParam> pParam = std::make_shared<MShaderConstantParam>();
        pParam->unSet                                = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);

        std::string uav_name = compiler.get_name(res.id);
        if (uav_name.empty())// compatible glslang.
        {
            uav_name = res.name;
        }

        pParam->strName = MStringId(uav_name.c_str());

        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

        BuildVariant(compiler, type, pParam->var);

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_params.push_back(pParam);
    }

    for (const spirv_cross::Resource& res: shaderResources.separate_images)
    {
        spirv_cross::SPIRType                type = compiler.get_type(res.type_id);

        std::shared_ptr<MShaderTextureParam> pParam = std::make_shared<MShaderTextureParam>();
        uint32_t                             nSet   = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t nBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);

        pParam->unSet     = nSet;
        pParam->unBinding = nBinding;
        pParam->strName   = MStringId(res.name.c_str());

        auto sub_type = compiler.get_type(type.image.type);

        if (type.image.dim == spv::Dim::Dim3D) { pParam->eType = METextureType::ETexture3D; }
        else if (type.image.dim == spv::Dim::DimCube) { pParam->eType = METextureType::ETextureCube; }
        else if (type.image.arrayed) { pParam->eType = METextureType::ETexture2DArray; }
        else if (type.image.dim == spv::Dim2D) { pParam->eType = METextureType::ETexture2D; }
        else { MORTY_ASSERT(false); }

        if (sub_type.basetype == spirv_cross::SPIRType::BaseType::Float) { pParam->eFormat = MESamplerFormat::EFloat; }
        else if (sub_type.basetype == spirv_cross::SPIRType::BaseType::UInt)
        {
            pParam->eFormat = MESamplerFormat::EInt;
        }
        else { MORTY_ASSERT(false); }

        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_textures.push_back(pParam);
    }

    for (const spirv_cross::Resource& res: shaderResources.storage_images)
    {
        spirv_cross::SPIRType                type = compiler.get_type(res.type_id);

        std::shared_ptr<MShaderTextureParam> pParam = std::make_shared<MShaderTextureParam>();
        uint32_t                             nSet   = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t nBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);

        pParam->unSet     = nSet;
        pParam->unBinding = nBinding;
        pParam->strName   = MStringId(res.name.c_str());

        if (type.image.dim == spv::Dim::Dim3D) { pParam->eType = METextureType::ETexture3D; }
        else if (type.image.dim == spv::Dim::DimCube) { pParam->eType = METextureType::ETextureCube; }
        else if (type.image.arrayed) { pParam->eType = METextureType::ETexture2DArray; }
        else if (type.image.dim == spv::Dim2D) { pParam->eType = METextureType::ETexture2D; }
        else { MORTY_ASSERT(false); }

        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_textures.push_back(pParam);
    }

    for (const spirv_cross::Resource& res: shaderResources.separate_samplers)
    {
        std::shared_ptr<MShaderSampleParam> pParam = std::make_shared<MShaderSampleParam>();
        pParam->unSet                              = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
        pParam->strName   = MStringId(res.name.c_str());

        if (!pParam->strName.ToString().empty())
        {
            if ('L' == pParam->strName.ToString()[0]) pParam->eSamplerType = MESamplerType::ELinear;
            else
                pParam->eSamplerType = MESamplerType::ENearest;
        }

        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_samples.push_back(pParam);
    }

    for (const spirv_cross::Resource& res: shaderResources.subpass_inputs)
    {
        std::shared_ptr<MShaderSubpasssInputParam> pParam = std::make_shared<MShaderSubpasssInputParam>();
        pParam->unSet     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        pParam->unBinding = compiler.get_decoration(res.id, spv::Decoration::DecorationBinding);
        pParam->strName   = MStringId(res.name.c_str());

        pParam->m_vkDescriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

        pShaderBuffer->m_shaderSets[pParam->unSet]->m_textures.push_back(pParam);
    }
}

void MVulkanShaderReflector::BuildVariant(
        const spirv_cross::Compiler& compiler,
        const spirv_cross::SPIRType& type,
        MVariant&                    variant
)
{
    MVariant tempVariant;

    if (spirv_cross::SPIRType::BaseType::Struct == type.basetype)
    {
        MVariantStruct        srt;
        MVariantStructBuilder builder(srt);
        for (uint32_t i = 0; i < type.member_types.size(); ++i)
        {
            const spirv_cross::TypeID&  id         = type.member_types[i];
            const spirv_cross::SPIRType memberType = compiler.get_type(id);

            spirv_cross::TypeID         typeId        = compiler.get_type(type.self).self;
            const std::string&          strMemberName = compiler.get_member_name(typeId, i);

            MVariant                    memberVariant;
            BuildVariant(compiler, memberType, memberVariant);
            builder.AppendVariant(MStringId(strMemberName.c_str()), memberVariant);
        }
        builder.Finish();
        tempVariant = MVariant(srt);
    }
    else { BuildBasicVariant(type, tempVariant); }

    if (type.array.empty()) { variant = std::move(tempVariant); }
    else
    {
        uint32_t             unArraySize = type.array[0];

        MVariantArray        varArray;
        MVariantArrayBuilder builder(varArray);

        for (uint32_t i = 0; i < unArraySize; ++i) { builder.AppendVariant(tempVariant); }
        builder.Finish();
        variant = MVariant(varArray);
    }
}

MVariant CreateFloatVariant(uint32_t nColumn, uint32_t nVecsize)
{
    switch (nColumn)
    {
        case 1: {
            switch (nVecsize)
            {
                case 1: return MVariant(float());
                case 2: return MVariant(Vector2());
                case 3: return MVariant(Vector3());
                case 4: return MVariant(Vector4());
                default: break;
            }
            break;
        }
        case 3: {
            if (3 == nVecsize) { return MVariant(Matrix3()); }
            break;
        }
        case 4: {
            if (4 == nVecsize) { return MVariant(Matrix4()); }
            break;
        }

        default: break;
    }

    MORTY_ASSERT(false);
    return {};
}

bool MVulkanShaderReflector::BuildBasicVariant(const spirv_cross::SPIRType& type, MVariant& variant) const
{
    //todo support intN, boolN and matrixNxM

    switch (type.basetype)
    {
        case spirv_cross::SPIRType::BaseType::Boolean: variant = MVariant(uint32_t()); return true;

        case spirv_cross::SPIRType::BaseType::Int: variant = MVariant(int()); return true;

        case spirv_cross::SPIRType::BaseType::UInt: variant = MVariant(uint32_t()); return true;

        case spirv_cross::SPIRType::BaseType::Float: {
            variant = CreateFloatVariant(type.columns, type.vecsize);
            return true;
        }
        default: MORTY_ASSERT(false); break;
    }

    m_device->GetEngine()->GetLogger()->Error("Can`t convert MVariant from spirv_cross::SPIRType. Unknow type");
    return false;
}

#endif