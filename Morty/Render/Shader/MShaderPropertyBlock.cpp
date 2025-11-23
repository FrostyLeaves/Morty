#include "MShaderPropertyBlock.h"
#include "RHI/Abstract/MIDevice.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Vulkan/MVulkanDevice.h"

#endif

using namespace morty;

MShaderPropertyBlock::MShaderPropertyBlock()
    : m_params()
    , m_textures()
    , m_samples()
    , m_unKey(0)
    , m_shaderProgram()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderPropertyBlock::MShaderPropertyBlock(IShaderProgram* pShaderProgram, const uint32_t& unKey)
    : m_params()
    , m_textures()
    , m_samples()
    , m_storages()
    , m_unKey(unKey)
    , m_shaderProgram(pShaderProgram)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderPropertyBlock::MShaderPropertyBlock(const MShaderPropertyBlock& other)
    : m_unKey(other.m_unKey)
    , m_shaderProgram(other.m_shaderProgram)
{
    m_params.resize(m_params.size());
    m_textures.resize(m_textures.size());
    m_samples.resize(m_samples.size());
    m_storages.resize(m_storages.size());

    for (auto& m_param: m_params) m_param = std::make_unique<MShaderConstantParam>(*m_param);

    for (auto& m_texture: m_textures) m_texture = std::make_unique<MShaderTextureParam>(*m_texture);

    for (auto& m_sample: m_samples) m_sample = std::make_unique<MShaderSampleParam>(*m_sample);

    for (auto& m_storage: m_storages) m_storage = std::make_unique<MShaderStorageParam>(*m_storage);
}

MShaderConstantParam* MShaderPropertyBlock::FindConstantParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_params)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

MShaderStorageParam* MShaderPropertyBlock::FindStorageParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_storages)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

MShaderTextureParam* MShaderPropertyBlock::FindTextureParam(const MStringId& strParamName)
{
    for (const auto& pParam: m_textures)
    {
        if (pParam->strName == strParamName) return pParam.get();
    }

    return nullptr;
}

bool MShaderPropertyBlock::SetTexture(const MStringId& strName, const MTexturePtr& pTexture)
{
    for (auto& pParam: m_textures)
    {
        if (pParam->strName == strName)
        {
            pParam->SetTexture(pTexture);
            return true;
        }
    }

    return false;
}

bool MShaderPropertyBlock::HasValue(const uint32_t& unBinding, const uint32_t& unSet)
{
    for (auto& pParam: m_params)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (auto& pParam: m_textures)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (auto& pParam: m_samples)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    return false;
}

void MShaderPropertyBlock::GenerateBuffer(MIDevice* pDevice) { pDevice->GenerateShaderPropertyBlock(this); }

void MShaderPropertyBlock::DestroyBuffer(MIDevice* pDevice)
{
    pDevice->DestroyShaderPropertyBlock(this);

    for (auto& pParam: m_params) { pDevice->DestroyShaderParamBuffer(pParam.get()); }
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::Clone() const
{
    auto propertyBlock = std::make_shared<MShaderPropertyBlock>(m_shaderProgram, m_unKey);

    propertyBlock->m_params.resize(m_params.size());
    propertyBlock->m_textures.resize(m_textures.size());
    propertyBlock->m_samples.resize(m_samples.size());
    propertyBlock->m_storages.resize(m_storages.size());

    for (uint32_t i = 0; i < m_params.size(); ++i)
        propertyBlock->m_params[i] = std::make_unique<MShaderConstantParam>(*m_params[i]);

    for (uint32_t i = 0; i < m_textures.size(); ++i) propertyBlock->m_textures[i] = m_textures[i]->Clone();

    for (uint32_t i = 0; i < m_samples.size(); ++i)
        propertyBlock->m_samples[i] = std::make_unique<MShaderSampleParam>(*m_samples[i]);

    for (uint32_t i = 0; i < m_storages.size(); ++i)
        propertyBlock->m_storages[i] = std::make_unique<MShaderStorageParam>(*m_storages[i]);

    return propertyBlock;
}
