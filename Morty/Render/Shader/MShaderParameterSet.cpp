#include "MShaderParameterSet.h"
#include "RHI/Abstract/MIDevice.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanDevice.h"

#endif

using namespace morty;

MShaderParameterSet::MShaderParameterSet()
    : m_uniforms()
    , m_textures()
    , m_samplers()
    , m_unKey(0)
    , m_shaderProgram()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderParameterSet::MShaderParameterSet(IShaderProgram* pShaderProgram, const uint32_t& unKey)
    : m_uniforms()
    , m_textures()
    , m_samplers()
    , m_storages()
    , m_unKey(unKey)
    , m_shaderProgram(pShaderProgram)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderParameterSet::MShaderParameterSet(const MShaderParameterSet& other)
    : m_unKey(other.m_unKey)
    , m_shaderProgram(other.m_shaderProgram)
{
    m_uniforms.resize(m_uniforms.size());
    m_textures.resize(m_textures.size());
    m_samplers.resize(m_samplers.size());
    m_storages.resize(m_storages.size());

    for (auto& m_param: m_uniforms) m_param = std::make_unique<MShaderUniformParam>(*m_param);

    for (auto& m_texture: m_textures) m_texture = std::make_unique<MShaderTextureParam>(*m_texture);

    for (auto& m_sample: m_samplers) m_sample = std::make_unique<MShaderSamplerParam>(*m_sample);

    for (auto& m_storage: m_storages) m_storage = std::make_unique<MShaderStorageParam>(*m_storage);
}

MShaderUniformParam* MShaderParameterSet::FindConstantParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_uniforms)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

MShaderStorageParam* MShaderParameterSet::FindStorageParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_storages)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

MShaderTextureParam* MShaderParameterSet::FindTextureParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_textures)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

MVariant FindValueRecursive(MVariant& variant, const MStringId& strName)
{
    if (variant.IsType<MVariantStruct>())
    {
        MVariant& findResult = variant.GetValue<MVariantStruct>().FindVariant(strName);
        if (findResult.IsValid()) { return findResult; }

        for (auto member: variant.GetValue<MVariantStruct>().GetMember())
        {
            MVariant& child = member.second;
            if (child.IsType<MVariantStruct>())
            {
                MVariant variant = FindValueRecursive(child, strName);
                if (variant.IsValid()) { return variant; }
            }
        }
    }

    return MVariant();
}

MVariant MShaderParameterSet::FindValue(const MStringId& strName, MShaderUniformParam** outputOwner)
{
    for (std::unique_ptr<MShaderUniformParam>& pParam: m_uniforms)
    {
        if (outputOwner) { *outputOwner = pParam.get(); }
        if (pParam->strName == strName) { return pParam->var; }
        else if (pParam->var.GetType() == MEVariantType::EStruct) { return FindValueRecursive(pParam->var, strName); }
    }

    return MVariant();
}

bool MShaderParameterSet::SetTexture(const MStringId& strName, const MTexturePtr& texture)
{
    for (auto& pParam: m_textures)
    {
        if (pParam->strName == strName)
        {
            pParam->SetTexture(texture);
            return true;
        }
    }

    return false;
}

bool MShaderParameterSet::HasValue(const uint32_t& unBinding, const uint32_t& unSet)
{
    for (auto& pParam: m_uniforms)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (auto& pParam: m_textures)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (auto& pParam: m_samplers)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    return false;
}

bool MShaderParameterSet::SetBuffer(const MStringId& name, const MBuffer* buffer)
{
    for (auto& param: m_storages)
    {
        if (param->strName == name)
        {
            param->SetBuffer(buffer);
            return true;
        }
    }

    return false;
}

void MShaderParameterSet::GenerateBuffer(MIDevice* pDevice) { pDevice->GenerateShaderParameterSet(this); }

void MShaderParameterSet::DestroyBuffer(MIDevice* pDevice)
{
    pDevice->DestroyShaderParameterSet(this);

    for (auto& pParam: m_uniforms) { pDevice->DestroyShaderParamBuffer(pParam.get()); }
}

std::shared_ptr<MShaderParameterSet> MShaderParameterSet::Clone() const
{
    auto propertyBlock = std::make_shared<MShaderParameterSet>(m_shaderProgram, m_unKey);

    propertyBlock->m_uniforms.resize(m_uniforms.size());
    propertyBlock->m_textures.resize(m_textures.size());
    propertyBlock->m_samplers.resize(m_samplers.size());
    propertyBlock->m_storages.resize(m_storages.size());

    for (uint32_t i = 0; i < m_uniforms.size(); ++i)
        propertyBlock->m_uniforms[i] = std::make_unique<MShaderUniformParam>(*m_uniforms[i]);

    for (uint32_t i = 0; i < m_textures.size(); ++i) propertyBlock->m_textures[i] = m_textures[i]->Clone();

    for (uint32_t i = 0; i < m_samplers.size(); ++i)
        propertyBlock->m_samplers[i] = std::make_unique<MShaderSamplerParam>(*m_samplers[i]);

    for (uint32_t i = 0; i < m_storages.size(); ++i)
        propertyBlock->m_storages[i] = std::make_unique<MShaderStorageParam>(*m_storages[i]);

    return propertyBlock;
}
