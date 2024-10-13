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

MShaderPropertyBlock::MShaderPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
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

    for (uint32_t i = 0; i < m_params.size(); ++i) m_params[i] = std::make_shared<MShaderConstantParam>(*m_params[i]);

    for (uint32_t i = 0; i < m_textures.size(); ++i)
        m_textures[i] = std::make_shared<MShaderTextureParam>(*m_textures[i]);

    for (uint32_t i = 0; i < m_samples.size(); ++i) m_samples[i] = std::make_shared<MShaderSampleParam>(*m_samples[i]);

    for (uint32_t i = 0; i < m_storages.size(); ++i)
        m_storages[i] = std::make_shared<MShaderStorageParam>(*m_storages[i]);
}

std::shared_ptr<MShaderConstantParam> MShaderPropertyBlock::FindConstantParam(const MStringId& strParamName)
{
    for (std::shared_ptr<MShaderConstantParam>& pParam: m_params)
    {
        if (pParam->strName == strParamName) return pParam;
    }

    return nullptr;
}

std::shared_ptr<MShaderStorageParam> MShaderPropertyBlock::FindStorageParam(const MStringId& strParamName)
{
    for (std::shared_ptr<MShaderStorageParam>& pParam: m_storages)
    {
        if (pParam->strName == strParamName) return pParam;
    }

    return nullptr;
}

std::shared_ptr<MShaderTextureParam> MShaderPropertyBlock::FindTextureParam(const MStringId& strParamName)
{
    for (std::shared_ptr<MShaderTextureParam>& pParam: m_textures)
    {
        if (pParam->strName == strParamName) return pParam;
    }

    return nullptr;
}

bool MShaderPropertyBlock::SetTexture(const MStringId& strName, MTexturePtr pTexture)
{
    for (std::shared_ptr<MShaderTextureParam>& pParam: m_textures)
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
    for (std::shared_ptr<MShaderConstantParam>& pParam: m_params)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (std::shared_ptr<MShaderTextureParam>& pParam: m_textures)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    for (std::shared_ptr<MShaderSampleParam>& pParam: m_samples)
    {
        if (pParam->unSet == unSet && pParam->unBinding == unBinding) return true;
    }

    return false;
}

void MShaderPropertyBlock::GenerateBuffer(MIDevice* pDevice) { pDevice->GenerateShaderPropertyBlock(GetShared()); }

void MShaderPropertyBlock::DestroyBuffer(MIDevice* pDevice)
{
    pDevice->DestroyShaderPropertyBlock(GetShared());

    for (std::shared_ptr<MShaderConstantParam>& pParam: m_params) { pDevice->DestroyShaderParamBuffer(pParam); }
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::Clone() const
{
    std::shared_ptr<MShaderPropertyBlock> pPropertyBlock =
            MShaderPropertyBlock::MakeShared(m_shaderProgram.lock(), m_unKey);

    pPropertyBlock->m_params.resize(m_params.size());
    pPropertyBlock->m_textures.resize(m_textures.size());
    pPropertyBlock->m_samples.resize(m_samples.size());
    pPropertyBlock->m_storages.resize(m_storages.size());

    for (uint32_t i = 0; i < m_params.size(); ++i)
        pPropertyBlock->m_params[i] = std::make_shared<MShaderConstantParam>(*m_params[i]);

    for (uint32_t i = 0; i < m_textures.size(); ++i) pPropertyBlock->m_textures[i] = m_textures[i]->Clone();

    for (uint32_t i = 0; i < m_samples.size(); ++i)
        pPropertyBlock->m_samples[i] = std::make_shared<MShaderSampleParam>(*m_samples[i]);

    for (uint32_t i = 0; i < m_storages.size(); ++i)
        pPropertyBlock->m_storages[i] = std::make_shared<MShaderStorageParam>(*m_storages[i]);

    return pPropertyBlock;
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::GetShared() const { return m_selfPointer.lock(); }

std::shared_ptr<MShaderPropertyBlock>
MShaderPropertyBlock::MakeShared(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
{
    std::shared_ptr<MShaderPropertyBlock> pResult = std::make_shared<MShaderPropertyBlock>(pShaderProgram, unKey);
    pResult->m_selfPointer                        = pResult;

    return pResult;
}

std::shared_ptr<MShaderPropertyBlock>
MShaderPropertyBlock::MakeShared(const std::shared_ptr<MShaderPropertyBlock>& other)
{
    std::shared_ptr<MShaderPropertyBlock> pResult = std::make_shared<MShaderPropertyBlock>(*other);
    pResult->m_selfPointer                        = pResult;

    return pResult;
}
