#include "Material/MShaderParamSet.h"
#include "Render/MIDevice.h"
#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanDevice.h"
#endif

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
		m_vParams[i] = std::make_shared<MShaderConstantParam>(*m_vParams[i], 0);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		m_vTextures[i] = std::make_shared<MShaderTextureParam>(*m_vTextures[i]);

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		m_vSamples[i] = std::make_shared<MShaderSampleParam>(*m_vSamples[i]);

	for (uint32_t i = 0; i < m_vStorages.size(); ++i)
		m_vStorages[i] = std::make_shared<MShaderStorageParam>(*m_vStorages[i]);
}

std::shared_ptr<MShaderConstantParam> MShaderPropertyBlock::FindConstantParam(const MString& strParamName)
{
	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderStorageParam> MShaderPropertyBlock::FindStorageParam(const MString& strParamName)
{
	for (std::shared_ptr<MShaderStorageParam>& pParam : m_vStorages)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderTextureParam> MShaderPropertyBlock::FindTextureParam(const MString& strParamName)
{
	for (std::shared_ptr<MShaderTextureParam>& pParam : m_vTextures)
	{
		if (pParam->strName == strParamName)
			return pParam;
	}

	return nullptr;
}

bool MShaderPropertyBlock::SetTexture(const MString& strName, std::shared_ptr<MTexture> pTexture)
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
	pDevice->GenerateShaderParamSet(GetShared());
}

void MShaderPropertyBlock::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyShaderParamSet(GetShared());

	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		pDevice->DestroyShaderParamBuffer(pParam);
	}
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::Clone() const
{
	std::shared_ptr<MShaderPropertyBlock> pParamSet = MShaderPropertyBlock::MakeShared(m_pShaderProgram.lock(), m_unKey);

	pParamSet->m_vParams.resize(m_vParams.size());
	pParamSet->m_vTextures.resize(m_vTextures.size());
	pParamSet->m_vSamples.resize(m_vSamples.size());
	pParamSet->m_vStorages.resize(m_vStorages.size());

	for (uint32_t i = 0; i < m_vParams.size(); ++i)
		pParamSet->m_vParams[i] = std::make_shared<MShaderConstantParam>(*m_vParams[i], 0);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		pParamSet->m_vTextures[i] = std::make_shared<MShaderTextureParam>(*m_vTextures[i]);

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		pParamSet->m_vSamples[i] = std::make_shared<MShaderSampleParam>(*m_vSamples[i]);
	
	for (uint32_t i = 0; i < m_vStorages.size(); ++i)
		pParamSet->m_vStorages[i] = std::make_shared<MShaderStorageParam>(*m_vStorages[i]);

	return pParamSet;
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
