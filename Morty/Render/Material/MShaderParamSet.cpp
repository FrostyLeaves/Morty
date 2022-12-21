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
	m_unLayoutDataIdx = MGlobal::M_INVALID_INDEX;
#endif
}

MShaderPropertyBlock::MShaderPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
	: m_vParams()
	, m_vTextures()
	, m_vSamples()
	, m_unKey(unKey)
	, m_pShaderProgram(pShaderProgram)
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	m_VkDescriptorSet = VK_NULL_HANDLE;
	m_unLayoutDataIdx = MGlobal::M_INVALID_INDEX;
#endif
}

MShaderPropertyBlock::~MShaderPropertyBlock()
{
	for (auto& pParam : m_vParams)
	{
		pParam = nullptr;
	}

	for (auto& pParam : m_vTextures)
	{
		pParam = nullptr;
	}

	for (auto& pParam : m_vSamples)
	{
		pParam = nullptr;
	}

	for (auto& pParam : m_vStorages)
	{
		pParam = nullptr;
	}

	m_vParams.clear();
	m_vTextures.clear();
	m_vSamples.clear();
	m_vStorages.clear();
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

MVariant* MShaderPropertyBlock::FindValue(const MString& strName)
{
	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		if (pParam->strName == strName)
			return &pParam->var;
		else if (MVariant* pVariant = FindValue(strName, pParam->var))
			return pVariant;
	}

	return nullptr;
}

MVariant* MShaderPropertyBlock::FindValue(const MString& strName, MVariant& value)
{
	if (MStruct* pStruct = value.GetStruct())
	{
		for (size_t i = 0; i < pStruct->GetMemberCount(); ++i)
		{
			if (MStruct::MContainer::MStructMember* pContainer = pStruct->GetMember(i))
			{
				if (pContainer->strName == strName)
					return &pContainer->var;
				else if (MVariant* pVariant = FindValue(strName, pContainer->var))
					return pVariant;
			}
		}
	}

	return nullptr;
}

bool MShaderPropertyBlock::SetValue(const MString& strName, MTexture* pTexture)
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

bool MShaderPropertyBlock::SetValue(const MString& strName, const MVariant& value)
{
	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		if (pParam->strName == strName)
		{
			if (SetValue(pParam->var, value))
			{
				pParam->SetDirty();
				return true;
			}
		}
		else if (MVariant* pVariant = FindValue(strName, pParam->var))
		{
			if (SetValue(*pVariant, value))
			{
				pParam->SetDirty();
				return true;
			}
		}
	}

	return false;
}

bool MShaderPropertyBlock::SetValue(MVariant& target, const MVariant& source)
{
	if (target.GetType() == source.GetType() && target.GetSize() == source.GetSize())
	{
		target = source;
		return true;
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
	pDevice->GenerateShaderParamSet(this);
}

void MShaderPropertyBlock::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyShaderParamSet(this);

	for (std::shared_ptr<MShaderConstantParam>& pParam : m_vParams)
	{
		pDevice->DestroyShaderParamBuffer(pParam);
	}
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::Clone()
{
	std::shared_ptr<MShaderPropertyBlock> pParamSet = MShaderPropertyBlock::MakeShared(m_pShaderProgram.lock(), m_unKey);

	pParamSet->m_vParams.resize(m_vParams.size());
	pParamSet->m_vTextures.resize(m_vTextures.size());
	pParamSet->m_vSamples.resize(m_vSamples.size());

	for (uint32_t i = 0; i < m_vParams.size(); ++i)
		pParamSet->m_vParams[i] = std::make_shared<MShaderConstantParam>(*m_vParams[i], 0);

	for (uint32_t i = 0; i < m_vTextures.size(); ++i)
		pParamSet->m_vTextures[i] = std::make_shared<MShaderTextureParam>(*m_vTextures[i]);

	for (uint32_t i = 0; i < m_vSamples.size(); ++i)
		pParamSet->m_vSamples[i] = std::make_shared<MShaderSampleParam>(*m_vSamples[i]);
	
	for (uint32_t i = 0; i < m_vStorages.size(); ++i)
		pParamSet->m_vStorages[i] = std::make_shared<MShaderStorageParam>(*m_vSamples[i]);

	return pParamSet;
}

std::shared_ptr<MShaderPropertyBlock> MShaderPropertyBlock::MakeShared(const std::shared_ptr<MShaderProgram>& pShaderProgram, const uint32_t& unKey)
{
	std::shared_ptr<MShaderPropertyBlock> pResult = std::make_shared<MShaderPropertyBlock>(pShaderProgram, unKey);
	pResult->m_pSelfPointer = pResult;

	return pResult;
}
