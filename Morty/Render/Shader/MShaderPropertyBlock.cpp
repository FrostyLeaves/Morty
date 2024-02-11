#include "MShaderPropertyBlock.h"
#include "Render/MIDevice.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanDevice.h"
#endif

using namespace morty;

MShaderPropertyBlock::MShaderPropertyBlock()
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(0)
	, m_pShaderProgram()
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderPropertyBlock::MShaderPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_vStorages()
	, m_unKey(unKey)
	, m_pShaderProgram(pShaderProgram)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorSet = VK_NULL_HANDLE;
#endif
}

MShaderPropertyBlock::MShaderPropertyBlock(const MShaderPropertyBlock& other)
	: m_unKey(other.m_unKey)
	, m_pShaderProgram(other.m_pShaderProgram)
{
	m_vParams.resize(m_vParams.size());
	m_vTextures.resize(m_vTextures.size());
	m_vSamples.resize(m_vSamples.size());
	m_vStorages.resize(m_vStorages.size());

	for (uint32_t i = 0; i < m_vParams.size(); ++i)
		m_vParams[i] = std::make_shared<MShaderConstantParam>(*m_vParams[i]);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		m_vTextures[i] = std::make_shared<MShaderTextureParam>(*m_vTextures[i]);

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		m_vSamples[i] = std::make_shared<MShaderSampleParam>(*m_vSamples[i]);

	for (uint32_t i = 0; i < m_vStorages.size(); ++i)
		m_vStorages[i] = std::make_shared<MShaderStorageParam>(*m_vStorages[i]);
}

std::shared_ptr<MShaderConstantParam> MShaderPropertyBlock::FindConstantParam(const MStringId& strParamName)
{
	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderStorageParam> MShaderPropertyBlock::FindStorageParam(const MStringId& strParamName)
{
	for (std::shared_ptr<MShaderStorageParam>& pParam : m_vStorages)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderTextureParam> MShaderPropertyBlock::FindTextureParam(const MStringId& strParamName)
{
	for (std::shared_ptr<MShaderTextureParam>& pParam : m_vTextures)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

bool MShaderPropertyBlock::SetTexture(const MStringId& strName, std::shared_ptr<MTexture> pTexture)
{
	for (std::shared_ptr<MShaderTextureParam>& pParam : m_vTextures)
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
	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	for (std::shared_ptr<MShaderTextureParam>& pParam : m_vTextures)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	for (std::shared_ptr<MShaderSampleParam>& pParam : m_vSamples)
	{
		if (pParam->unSet == unSet && pParam->unBinding == unBinding)
			return true;
	}

	return false;
}

void MShaderPropertyBlock::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->GenerateShaderPropertyBlock(GetShared());
}

void MShaderPropertyBlock::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyShaderPropertyBlock(GetShared());

	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		pDevice->DestroyShaderParamBuffer(pParam);
	}
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::Clone() const
{
	std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = MShaderPropertyBlock::MakeShared(m_pShaderProgram.lock(), m_unKey);

	pPropertyBlock->m_vParams.resize(m_vParams.size());
	pPropertyBlock->m_vTextures.resize(m_vTextures.size());
	pPropertyBlock->m_vSamples.resize(m_vSamples.size());
	pPropertyBlock->m_vStorages.resize(m_vStorages.size());

	for (uint32_t i = 0; i < m_vParams.size(); ++i)
		pPropertyBlock->m_vParams[i] = std::make_shared<MShaderConstantParam>(*m_vParams[i]);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		pPropertyBlock->m_vTextures[i] = m_vTextures[i]->Clone();

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		pPropertyBlock->m_vSamples[i] = std::make_shared<MShaderSampleParam>(*m_vSamples[i]);
	
	for (uint32_t i = 0; i < m_vStorages.size(); ++i)
		pPropertyBlock->m_vStorages[i] = std::make_shared<MShaderStorageParam>(*m_vStorages[i]);

	return pPropertyBlock;
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::GetShared() const
{
	return m_pSelfPointer.lock();
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::MakeShared(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
{
	std::shared_ptr<MShaderPropertyBlock> pResult = std::make_shared<MShaderPropertyBlock>(pShaderProgram, unKey);
	pResult->m_pSelfPointer = pResult;

	return pResult;
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::MakeShared(const std::shared_ptr<MShaderPropertyBlock>& other)
{
	std::shared_ptr<MShaderPropertyBlock> pResult = std::make_shared<MShaderPropertyBlock>(*other);
	pResult->m_pSelfPointer = pResult;

	return pResult;
}
